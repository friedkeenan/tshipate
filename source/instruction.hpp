#pragma once

#include "common.hpp"
#include "bit_pattern.hpp"

namespace tsh {

    /* Forward declare. */
    class Chip8;

    using RawOpcode = std::uint16_t;

    class Opcode {
        public:
            RawOpcode op;

            ALWAYS_INLINE constexpr Opcode(RawOpcode op) : op(op) { }

            ALWAYS_INLINE constexpr const RawOpcode &Get() const {
                return this->op;
            }

            ALWAYS_INLINE constexpr std::uint16_t Addr() const {
                return this->Get() & 0x0FFF;
            }

            ALWAYS_INLINE constexpr std::uint8_t X() const {
                return (this->Get() & 0x0F00) >> 8;
            }

            ALWAYS_INLINE constexpr std::uint8_t Y() const {
                return (this->Get() & 0x00F0) >> 4;
            }

            ALWAYS_INLINE constexpr std::uint8_t Byte() const {
                return this->Get() & 0x00FF;
            }

            ALWAYS_INLINE constexpr std::uint8_t Nibble() const {
                return this->Get() & 0x000F;
            }
    };

    static_assert(sizeof(Opcode) == sizeof(std::uint16_t));

    template<typename T>
    concept Instruction = requires(Chip8 ch8, Opcode op) {
        { T::Compare(op) }      -> std::same_as<bool>;
        { T::Execute(ch8, op) } -> std::same_as<std::int32_t>;
    };

    template<Instruction Ins, typename... Ts>
    class InstructionHandler {
        public:
            static constexpr std::optional<std::int32_t> Execute(Chip8 &ch8, Opcode op) {
                if (Ins::Compare(op)) {
                    return Ins::Execute(ch8, op);
                }

                return InstructionHandler<Ts...>::Execute(ch8, op);
            }
    };

    /* Base case. */
    template<Instruction Ins>
    class InstructionHandler<Ins> {
        public:
            static constexpr std::optional<std::int32_t> Execute(Chip8 &ch8, Opcode op) {
                if (Ins::Compare(op)) {
                    return Ins::Execute(ch8, op);
                }

                return {};
            }
    };

    #define INSTRUCTION_DECLARE(name, pattern)                           \
        class name {                                                     \
            public:                                                      \
                static constexpr auto Pattern = BitPattern(pattern);     \
                ALWAYS_INLINE static constexpr bool Compare(Opcode op) { \
                    return Pattern.matches(op.Get());                    \
                }                                                        \
                static std::int32_t Execute(Chip8 &ch8, Opcode op);      \
        }

    INSTRUCTION_DECLARE(RET,        "0000'0000'1110'1110");
    INSTRUCTION_DECLARE(JP_Addr,    "0001'xxxx'xxxx'xxxx");
    INSTRUCTION_DECLARE(CALL,       "0010'xxxx'xxxx'xxxx");
    INSTRUCTION_DECLARE(LD_Byte,    "0110'xxxx'xxxx'xxxx");
    INSTRUCTION_DECLARE(ADD_Byte,   "0111'xxxx'xxxx'xxxx");
    INSTRUCTION_DECLARE(LD_Addr,    "1010'xxxx'xxxx'xxxx");

    #undef INSTRUCTION_DECLARE

}
