#pragma once

#include "common.hpp"
#include "util.hpp"
#include "instruction.hpp"
#include "digits.hpp"

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

            constexpr Internal Max() const {
                return std::numeric_limits<Internal>::max();
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr const Internal &Get() const {
                return this->value;
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr bool IsBitSet(const std::uint8_t bit) const {
                return (this->value & (Internal{1} << bit)) != 0;
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
            using Coord = std::uint8_t;

            static constexpr Coord DisplayWidth  = 64;
            static constexpr Coord DisplayHeight = 32;

            /* Other widths are not supported. */
            static_assert(DisplayWidth == 64);

            static constexpr auto WindowWidth  = DisplayWidth  * 10;
            static constexpr auto WindowHeight = DisplayHeight * 10;

            static constexpr auto PixelWidth = static_cast<float>(WindowWidth) / DisplayWidth;

            using RowType = std::uint64_t;

            static constexpr auto FullRow = std::numeric_limits<RowType>::max();

            static constexpr RowType XBit(const Coord x) {
                return (RowType{1} << (DisplayWidth - x - 1));
            }

            /* Bitmap representing on/off pixels. */
            std::array<RowType, DisplayHeight> buffer = {};

            ALWAYS_INLINE constexpr Display() = default;

            ALWAYS_INLINE constexpr void Clear() {
                std::ranges::fill(this->buffer, RowType{});
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr bool GetPixel(const Coord x, const Coord y) const {
                return (this->buffer[y] & XBit(x)) != 0;
            }

            ALWAYS_INLINE constexpr void SetPixel(const Coord x, const Coord y, const bool on) {
                if (on) {
                    this->buffer[y] |=  XBit(x);
                } else {
                    this->buffer[y] &= ~XBit(x);
                }
            }

            ALWAYS_INLINE constexpr void TogglePixel(const Coord x, const Coord y) {
                this->buffer[y] ^= XBit(x);
            }

            constexpr bool DrawSprite(const Coord x, Coord y, const std::span<const std::byte> data) {
                bool collide = false;

                for (const auto &byte : data) {
                    auto sprite_row = std::to_integer<RowType>(byte);

                    /*
                        Coords cannot be negative so only need to
                        consider overflowing over the right and bottom.
                    */
                    if (x + BITSIZEOF(std::byte) > DisplayWidth) {
                        const auto overflow_width = BITSIZEOF(std::byte) - (DisplayWidth - x);
                        const auto overflow       = (sprite_row & ~(FullRow << overflow_width));

                        sprite_row >>= overflow_width;
                        sprite_row  |= (overflow << (DisplayWidth - overflow_width));
                    } else {
                        sprite_row <<= (DisplayWidth - x - BITSIZEOF(std::byte));
                    }

                    auto &current_row = this->buffer[y];

                    const auto new_row = current_row ^ sprite_row;
                    if ((current_row | sprite_row) != new_row) {
                        collide = true;
                    }

                    current_row = new_row;

                    y = (y + 1) % DisplayHeight;
                }

                return collide;
            }

            ALWAYS_INLINE sf::RenderWindow OpenWindow() const {
                return sf::RenderWindow(sf::VideoMode(WindowWidth, WindowHeight), "tshipate");
            }

            bool Render(sf::RenderWindow &window) const;
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

        Invalid = 0x10,
    };

    class Keyboard {
        NON_COPYABLE(Keyboard);
        NON_MOVEABLE(Keyboard);

        public:
            static constexpr auto KeyToInternal = util::Map(
                sf::Keyboard::Unknown,

                std::pair{Key::Zero,  sf::Keyboard::Numpad0},
                std::pair{Key::One,   sf::Keyboard::Numpad7},
                std::pair{Key::Two,   sf::Keyboard::Numpad8},
                std::pair{Key::Three, sf::Keyboard::Numpad9},
                std::pair{Key::Four,  sf::Keyboard::Numpad4},
                std::pair{Key::Five,  sf::Keyboard::Numpad5},
                std::pair{Key::Six,   sf::Keyboard::Numpad6},
                std::pair{Key::Seven, sf::Keyboard::Numpad1},
                std::pair{Key::Eight, sf::Keyboard::Numpad2},
                std::pair{Key::Nine,  sf::Keyboard::Numpad3}
            );

            Key current_key = Key::Invalid;

            ALWAYS_INLINE constexpr Keyboard() = default;

            [[nodiscard]]
            ALWAYS_INLINE bool IsKeyPressed(const Key key) const {
                return sf::Keyboard::isKeyPressed(KeyToInternal[key]);
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr Key CurrentKey() {
                ON_SCOPE_EXIT { this->current_key = Key::Invalid; };

                return this->current_key;
            }

            bool HandleEvent(const sf::Event &event);
    };

    class Speaker {
        NON_COPYABLE(Speaker);
        NON_MOVEABLE(Speaker);

        public:
            ALWAYS_INLINE constexpr Speaker() = default;

            ALWAYS_INLINE constexpr void PlaySound() { }
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
            static constexpr auto DigitSpace   = AddressSpace(0x0000, sizeof(Digits));
            static constexpr auto ProgramSpace = AddressSpace(0x0200, 0x1000);

            static constexpr auto FrameDuration = std::chrono::duration<double>(1.0 / 60);

            using Handler = InstructionHandler<
                CLS,
                RET,
                JP_Addr,
                CALL,
                SE_V_Byte,
                SNE_V_Byte,
                LD_V_Byte,
                ADD_V_Byte,
                LD_V_V,
                AND_V_V,
                XOR_V_V,
                ADD_V_V,
                SUB_V_V,
                SHL_V,
                SNE_V_V,
                LD_I_Addr,
                RND,
                DRW,
                SKP,
                SKNP,
                LD_V_DT,
                LD_V_K,
                LD_DT_V,
                LD_ST_V,
                ADD_I_V,
                LD_F_V,
                LD_B_V,
                LD_DEREF_I_V,
                LD_V_DEREF_I
            >;

            std::array<std::byte, TotalSpace.Size()> memory = {};

            Display display;

            /* General purpose registers. */
            std::array<Register<std::uint8_t>, 0x10> V = {};

            /* Address register. */
            Register<Address> I;

            /* Program counter. */
            Register<Address> PC = ProgramSpace.start;

            /* Delay timer. */
            Timer<std::uint8_t> DT;

            /* Sound timer. */
            Timer<std::uint8_t> ST;

            std::stack<Address> stack;

            Keyboard keyboard;

            [[no_unique_address]] Speaker speaker;

            RandomGenerator rng;

            ALWAYS_INLINE Chip8() {
                for (const auto &&[i, digit] : util::enumerate(Digits)) {
                    std::ranges::copy(digit, this->memory.begin() + DigitSpace.start + (i * sizeof(Digit)));
                }
            }

            bool PropagateEvent(const sf::Event &event);

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
