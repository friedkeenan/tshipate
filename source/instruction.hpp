#pragma once

#include "common.hpp"
#include "nibble_pattern.hpp"

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

    static_assert(sizeof(Opcode) == sizeof(RawOpcode));

    template<typename T>
    concept Instruction = requires(Chip8 ch8, Opcode op) {
        { T::Compare(op) }      -> std::same_as<bool>;
        { T::Execute(ch8, op) } -> std::same_as<std::int32_t>;
    };

    template<Instruction Ins, typename... Ts>
    class InstructionHandler {
        public:
            static constexpr std::optional<std::int32_t> Execute(Chip8 &ch8, const Opcode op) {
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
            static constexpr std::optional<std::int32_t> Execute(Chip8 &ch8, const Opcode op) {
                if (Ins::Compare(op)) {
                    return Ins::Execute(ch8, op);
                }

                return {};
            }
    };

    #define INSTRUCTION_DECLARE(name, pattern)                                 \
        class name {                                                           \
            public:                                                            \
                static constexpr auto Pattern = NibblePattern(pattern);        \
                ALWAYS_INLINE static constexpr bool Compare(const Opcode op) { \
                    return Pattern.matches(op.Get());                          \
                }                                                              \
                static std::int32_t Execute(Chip8 &ch8, const Opcode op);      \
        }

    INSTRUCTION_DECLARE(RET,        "00EE");
    INSTRUCTION_DECLARE(JP_Addr,    "1xxx");
    INSTRUCTION_DECLARE(CALL,       "2xxx");
    INSTRUCTION_DECLARE(SE_V_Byte,  "3xxx");
    INSTRUCTION_DECLARE(SNE_V_Byte, "4xxx");
    INSTRUCTION_DECLARE(LD_V_Byte,  "6xxx");
    INSTRUCTION_DECLARE(ADD_V_Byte, "7xxx");
    INSTRUCTION_DECLARE(LD_V_V,     "8xx0");
    INSTRUCTION_DECLARE(SNE_V_V,    "9xx0");
    INSTRUCTION_DECLARE(LD_I_Addr,  "Axxx");
    INSTRUCTION_DECLARE(RND,        "Cxxx");
    INSTRUCTION_DECLARE(DRW,        "Dxxx");
    INSTRUCTION_DECLARE(SKP,        "Ex9E");
    INSTRUCTION_DECLARE(SKNP,       "ExA1");
    INSTRUCTION_DECLARE(LD_V_DT,    "Fx07");
    INSTRUCTION_DECLARE(LD_DT_V,    "Fx15");
    INSTRUCTION_DECLARE(ADD_I_V,    "Fx1E");

    #undef INSTRUCTION_DECLARE

}
