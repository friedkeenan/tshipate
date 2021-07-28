#pragma once

#include "common.hpp"
#include "instruction.hpp"

namespace tsh {

    using Address   = std::uint16_t;
    using RawOpcode = std::uint16_t;

    class AddressSpace {
        public:
            Address start, end;

            consteval AddressSpace(const Address start, const Address end) : start(start), end(end) {
                if (end < start) {
                    ERROR("End address must be after start address");
                }
            }

            [[nodiscard]]
            consteval std::size_t Size() const {
                return end - start;
            }
    };

    template<std::integral Internal>
    class Register {
        NON_COPYABLE(Register);
        NON_MOVEABLE(Register);

        public:
            Internal value = {};

            ALWAYS_INLINE constexpr Register() = default;

            ALWAYS_INLINE constexpr Register(const Internal value) : value(value) { }

            [[nodiscard]]
            ALWAYS_INLINE constexpr Internal Get() const {
                return this->value;
            }

            ALWAYS_INLINE constexpr void Set(const Internal value) {
                this->value = value;
            }

            template<std::integral T>
            ALWAYS_INLINE constexpr void Increment(const T delta) {
                this->value += delta;
            }

            template<std::integral T>
            ALWAYS_INLINE constexpr void Decrement(const T delta) {
                this->value -= delta;
            }
    };

    template<std::integral Internal>
    class Timer {
        NON_COPYABLE(Timer);
        NON_MOVEABLE(Timer);

        public:
            std::atomic<Internal> value = {};

            std::jthread thread;

            ALWAYS_INLINE Timer() = default;

            [[nodiscard]]
            ALWAYS_INLINE Internal Get() {
                return this->value.load();
            }

            ALWAYS_INLINE void Set(const Internal value) {
                this->value.store(value);
            }

            ALWAYS_INLINE void Increment(const Internal delta) {
                this->value += delta;
            }

            ALWAYS_INLINE void Decrement(const Internal delta) {
                this->value -= delta;
            }

            template<typename... Args>
            ALWAYS_INLINE void StartThread(Args &&... args) {
                this->thread = std::jthread(std::forward<Args>(args)...);
            }

            ALWAYS_INLINE bool RequestStop() {
                return this->thread.request_stop();
            }

            ALWAYS_INLINE void Join() {
                this->thread.join();
            }
    };

    class Display {
        NON_COPYABLE(Display);
        NON_MOVEABLE(Display);

        public:
            static constexpr std::size_t Width  = 64;
            static constexpr std::size_t Height = 32;

            /* Other widths are not supported. */
            static_assert(Width == 64);

            using Coord = std::uint8_t;
            using RowType = std::uint64_t;

            static constexpr auto FullRow = std::numeric_limits<RowType>::max();

            /* Bitmap representing on/off pixels. */
            std::array<RowType, Height> buffer = {};

            ALWAYS_INLINE constexpr Display() = default;

            ALWAYS_INLINE constexpr void Clear() {
                std::ranges::fill(this->buffer, RowType{});
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr bool GetPixel(const Coord x, const Coord y) const {
                return (this->buffer[y] & (RowType{1} << x)) != 0;
            }

            ALWAYS_INLINE constexpr void SetPixel(const Coord x, const Coord y, const bool on) {
                if (on) {
                    this->buffer[y] |=  (RowType{1} << x);
                } else {
                    this->buffer[y] &= ~(RowType{1} << x);
                }
            }

            ALWAYS_INLINE constexpr void TogglePixel(const Coord x, const Coord y) {
                this->buffer[y] ^= (RowType{1} << x);
            }

            constexpr bool DrawSprite(const Coord x, Coord y, const std::span<const std::byte> data) {
                bool collide = false;

                for (const auto &byte : data) {
                    auto sprite_row = std::to_integer<std::uint64_t>(byte);

                    /*
                        Coords cannot be negative so only need to
                        consider overflowing over the right and bottom.
                    */
                    if (x + BITSIZEOF(std::byte) > Width) {
                        const auto overflow_width = Width - x;
                        const auto overflow       = (sprite_row & ~(FullRow << overflow_width));

                        sprite_row >>= overflow_width;
                        sprite_row  |= (overflow << (BITSIZEOF(RowType) - overflow_width));
                    }

                    auto &current_row = this->buffer[y];

                    const auto new_row = current_row ^ sprite_row;
                    if ((current_row | sprite_row) != new_row) {
                        collide = true;
                    }

                    current_row = new_row;

                    y = (y + 1) % Height;
                }

                return collide;
            }
    };

    enum class Key : std::uint8_t {
        Zero  = 0x0,
        One   = 0x1,
        Two   = 0x2,
        Three = 0x3,
        Four  = 0x4,
        Five  = 0x5,
        Six   = 0x6,
        Seven = 0x7,
        Eight = 0x8,
        Nine  = 0x9,
        A     = 0xA,
        B     = 0xB,
        C     = 0xC,
        D     = 0xD,
        E     = 0xE,
        F     = 0xF,
    };

    class Keyboard {
        NON_COPYABLE(Keyboard);
        NON_MOVEABLE(Keyboard);

        public:
            ALWAYS_INLINE constexpr Keyboard() = default;

            [[nodiscard]]
            ALWAYS_INLINE constexpr bool KeyPressed(const Key key) const {
                UNUSED(key);

                return false;
            }
    };

    class RandomGenerator {
        NON_COPYABLE(RandomGenerator);
        NON_MOVEABLE(RandomGenerator);

        public:
            std::random_device rd;

            std::uniform_int_distribution<std::uint8_t> u8_dist;

            ALWAYS_INLINE RandomGenerator() : u8_dist(0, 255) { }

            ALWAYS_INLINE std::uint8_t RandomU8() {
                return this->u8_dist(rd);
            }
    };

    class Chip8 {
        NON_COPYABLE(Chip8);
        NON_MOVEABLE(Chip8);

        public:
            static constexpr auto TotalSpace   = AddressSpace(0x0000, 0x1000);
            static constexpr auto ProgramSpace = AddressSpace(0x0200, 0x1000);

            using Handler = InstructionHandler<
                RET,
                JP_Addr,
                CALL,
                SE_Byte,
                SNE_Byte,
                LD_Byte,
                ADD_Byte,
                LD_Addr,
                RND,
                DRW,
                SKP,
                SKNP,
                LD_V_DT,
                LD_DT_V,
                ADD_I_V
            >;

            std::array<std::byte, TotalSpace.Size()> memory = {};

            Display display;

            [[no_unique_address]] Keyboard keyboard;

            /* General purpose registers. */
            std::array<Register<std::uint8_t>, 0x10> V = {};

            /* Address register. */
            Register<Address> I;

            /* Program counter. */
            Register<Address> PC = ProgramSpace.start;

            /* Delay timer. */
            Timer<std::uint8_t> DT;

            std::stack<Address> stack;

            RandomGenerator rng;

            ALWAYS_INLINE Chip8() = default;

            [[nodiscard]]
            bool LoadProgram(const std::span<const std::byte> data);

            [[nodiscard]]
            bool LoadProgram(const std::string &path);

            constexpr RawOpcode ReadRawOpcode() const {
                return (
                    (std::to_integer<RawOpcode>(this->memory[this->PC.Get()]) << 8) |
                    (std::to_integer<RawOpcode>(this->memory[this->PC.Get() + 1]))
                );
            }

            [[nodiscard]]
            bool Tick();

            void Loop();
    };

}
