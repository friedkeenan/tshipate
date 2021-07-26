#pragma once

#include "common.hpp"

namespace tsh {

    using Address = std::uint16_t;

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

            std::array<std::byte, TotalSpace.Size()> memory = {};

            /* General purpose registers. */
            std::array<Register<std::uint8_t>, 0x10> V = {};

            /* Address register. */
            Register<Address> I;

            /* Program counter. */
            Register<Address> PC;

            ALWAYS_INLINE constexpr Chip8() = default;

            [[nodiscard]]
            bool LoadProgram(std::span<std::byte> data);

            [[nodiscard]]
            bool LoadProgram(const std::string &path);

            [[nodiscard]]
            bool Tick();

            void Loop();
    };

}
