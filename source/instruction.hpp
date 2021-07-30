#pragma once

#include "common.hpp"
#include "nibble_pattern.hpp"

namespace tsh {

    /* Forward declare. */
    class Chip8;

    class Opcode {
        public:
            RawOpcode op;

            ALWAYS_INLINE constexpr Opcode(RawOpcode op) : op(op) { }

            [[nodiscard]]
            ALWAYS_INLINE constexpr const RawOpcode &Get() const {
                return this->op;
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr Address Addr() const {
                return this->Get() & 0x0FFF;
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr std::uint8_t X() const {
                return (this->Get() & 0x0F00) >> 8;
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr std::uint8_t Y() const {
                return (this->Get() & 0x00F0) >> 4;
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr std::uint8_t Byte() const {
                return this->Get() & 0x00FF;
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr std::uint8_t Nibble() const {
                return this->Get() & 0x000F;
            }
    };

    static_assert(sizeof(Opcode) == sizeof(RawOpcode));

    using PCAdvance = std::int32_t;

    template<typename T>
    concept Instruction = requires(Chip8 &ch8, Opcode op) {
        { T::Compare(op) }      -> std::same_as<bool>;
        { T::Execute(ch8, op) } -> std::same_as<PCAdvance>;
    };

    template<Instruction Ins, typename... Ts>
    class InstructionHandler {
        public:
            [[nodiscard]]
            static constexpr std::optional<PCAdvance> Execute(Chip8 &ch8, const Opcode op) {
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
            [[nodiscard]]
            static constexpr std::optional<PCAdvance> Execute(Chip8 &ch8, const Opcode op) {
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
                [[nodiscard]]                                                  \
                ALWAYS_INLINE static constexpr bool Compare(const Opcode op) { \
                    return Pattern.matches(op.Get());                          \
                }                                                              \
                [[nodiscard]]                                                  \
                static PCAdvance Execute(Chip8 &ch8, const Opcode op);         \
        }

    INSTRUCTION_DECLARE(CLS,          "00E0");
    INSTRUCTION_DECLARE(RET,          "00EE");
    INSTRUCTION_DECLARE(JP_Addr,      "1xxx");
    INSTRUCTION_DECLARE(CALL,         "2xxx");
    INSTRUCTION_DECLARE(SE_V_Byte,    "3xxx");
    INSTRUCTION_DECLARE(SNE_V_Byte,   "4xxx");
    INSTRUCTION_DECLARE(SE_V_V,       "5xx0");
    INSTRUCTION_DECLARE(LD_V_Byte,    "6xxx");
    INSTRUCTION_DECLARE(ADD_V_Byte,   "7xxx");
    INSTRUCTION_DECLARE(LD_V_V,       "8xx0");
    INSTRUCTION_DECLARE(OR_V_V,       "8xx1");
    INSTRUCTION_DECLARE(AND_V_V,      "8xx2");
    INSTRUCTION_DECLARE(XOR_V_V,      "8xx3");
    INSTRUCTION_DECLARE(ADD_V_V,      "8xx4");
    INSTRUCTION_DECLARE(SUB_V_V,      "8xx5");
    INSTRUCTION_DECLARE(SHR_V,        "8xx6");
    INSTRUCTION_DECLARE(SUBN_V_V,     "8xx7");
    INSTRUCTION_DECLARE(SHL_V,        "8xxE");
    INSTRUCTION_DECLARE(SNE_V_V,      "9xx0");
    INSTRUCTION_DECLARE(LD_I_Addr,    "Axxx");
    INSTRUCTION_DECLARE(JP_V0_Addr,   "Bxxx");
    INSTRUCTION_DECLARE(RND,          "Cxxx");
    INSTRUCTION_DECLARE(DRW,          "Dxxx");
    INSTRUCTION_DECLARE(SKP,          "Ex9E");
    INSTRUCTION_DECLARE(SKNP,         "ExA1");
    INSTRUCTION_DECLARE(LD_V_DT,      "Fx07");
    INSTRUCTION_DECLARE(LD_V_K,       "Fx0A");
    INSTRUCTION_DECLARE(LD_DT_V,      "Fx15");
    INSTRUCTION_DECLARE(LD_ST_V,      "Fx18");
    INSTRUCTION_DECLARE(ADD_I_V,      "Fx1E");
    INSTRUCTION_DECLARE(LD_F_V,       "Fx29");
    INSTRUCTION_DECLARE(LD_B_V,       "Fx33");
    INSTRUCTION_DECLARE(LD_DEREF_I_V, "Fx55");
    INSTRUCTION_DECLARE(LD_V_DEREF_I, "Fx65");

    #undef INSTRUCTION_DECLARE

}
