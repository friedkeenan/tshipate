#pragma once

#include "common.hpp"
#include "instruction.hpp"

namespace tsh {

    using Address   = std::uint16_t;
    using RawOpcode = std::uint16_t;

    class AddressSpace {
        public:
            Address start, end;

            consteval AddressSpace(Address start, Address end) : start(start), end(end) {
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

            ALWAYS_INLINE constexpr Register(Internal value) : value(value) { }

            [[nodiscard]]
            ALWAYS_INLINE constexpr const Internal &Get() const {
                return this->value;
            }

            ALWAYS_INLINE constexpr void Set(Internal value) {
                this->value = value;
            }

            template<std::integral T>
            ALWAYS_INLINE constexpr void Increment(T delta) {
                this->value += delta;
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
                LD_Byte,
                ADD_Byte,
                LD_Addr
            >;

            std::array<std::byte, TotalSpace.Size()> memory = {};

            /* General purpose registers. */
            std::array<Register<std::uint8_t>, 0x10> V = {};

            /* Address register. */
            Register<Address> I;

            /* Program counter. */
            Register<Address> PC = ProgramSpace.start;

            std::stack<Address> stack;

            ALWAYS_INLINE Chip8() = default;

            [[nodiscard]]
            bool LoadProgram(std::span<const std::byte> data);

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
