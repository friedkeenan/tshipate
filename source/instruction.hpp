#pragma once

#include <cstdint>
#include <concepts>
#include <tuple>

#include "defines.hpp"

namespace tsh {

    /* Forward declare */
    class Chip8;

    template<typename T> requires std::integral<T>
    class Opcode {
        public:
            const T op;

            constexpr ALWAYS_INLINE Opcode(const T &op) : op(op) { }

            constexpr ALWAYS_INLINE const T &Get() const {
                return this->op;
            }

            constexpr ALWAYS_INLINE std::uint16_t Addr() const {
                return this->Get() & 0x0fff;
            }

            constexpr ALWAYS_INLINE std::uint8_t X() const {
                return (this->Get() & 0x0f00) >> 8;
            }

            constexpr ALWAYS_INLINE std::uint8_t Y() const {
                return (this->Get() & 0x00f0) >> 4;
            }

            constexpr ALWAYS_INLINE std::uint8_t Byte() const {
                return this->Get() & 0x00ff;
            }

            constexpr ALWAYS_INLINE std::uint8_t Nibble() const {
                return this->Get() & 0x000f;
            }
    };

    template<typename T>
    concept HasOpcodeType = requires() {
        typename T::OpcodeType;
    };

    template<typename T>
    concept IsInstruction = HasOpcodeType<T> && requires(Chip8 &ch8, const Opcode<typename T::OpcodeType> &op) {
        { T::Compare(op) }      -> std::same_as<bool>;
        { T::Execute(ch8, op) } -> std::same_as<std::int64_t>;
    };

    template<typename T, typename... Ts>
    concept SameOpcodeType = HasOpcodeType<T> && (sizeof...(Ts) == 0 || ((HasOpcodeType<Ts> && ...) && (std::same_as<typename T::OpcodeType, typename Ts::OpcodeType> && ...)));

    template<typename T, typename... Ts> requires IsInstruction<T> && SameOpcodeType<T, Ts...>
    class InstructionHandler {
        public:
            using OpcodeType = T::OpcodeType;

            static constexpr std::int64_t Execute(Chip8 &ch8, const Opcode<OpcodeType> &op) {
                if (T::Compare(op)) {
                    return T::Execute(ch8, op);
                }

                return InstructionHandler<Ts...>::Execute(ch8, op);
            }
    };

    /* Base case */
    template<typename T> requires IsInstruction<T>
    class InstructionHandler<T> {
        public:
            using OpcodeType = T::OpcodeType;

            static constexpr std::int64_t Execute(Chip8 &ch8, const Opcode<OpcodeType> &op) {
                if (T::Compare(op)) {
                    return T::Execute(ch8, op);
                }

                return -1;
            }
    };

    #define INSTRUCTION_DECLARE(name, mask, compare)                                         \
        class name {                                                                         \
            public:                                                                          \
                using OpcodeType = std::uint16_t;                                            \
                static constexpr ALWAYS_INLINE bool Compare(const Opcode<OpcodeType> &op)  { \
                    return (op.Get() & mask) == compare;                                     \
                }                                                                            \
                static std::int64_t Execute(Chip8 &ch8, const Opcode<OpcodeType> &op);       \
        }

    INSTRUCTION_DECLARE(CLR,        0xffff, 0x00e0);
    INSTRUCTION_DECLARE(RTS,        0xffff, 0x00ee);
    INSTRUCTION_DECLARE(JP_Addr,    0xf000, 0x1000);
    INSTRUCTION_DECLARE(CALL,       0xf000, 0x2000);
    INSTRUCTION_DECLARE(SE_Byte,    0xf000, 0x3000);
    INSTRUCTION_DECLARE(SNE_Byte,   0xf000, 0x4000);
    INSTRUCTION_DECLARE(SE_Reg,     0xf00f, 0x5000);
    INSTRUCTION_DECLARE(LD_Byte,    0xf000, 0x6000);
    INSTRUCTION_DECLARE(ADD_Byte,   0xf000, 0x7000);
    INSTRUCTION_DECLARE(LD_V,       0xf00f, 0x8000);
    INSTRUCTION_DECLARE(OR,         0xf00f, 0x8001);
    INSTRUCTION_DECLARE(AND,        0xf00f, 0x8002);
    INSTRUCTION_DECLARE(XOR,        0xf00f, 0x8003);
    INSTRUCTION_DECLARE(ADD_V,      0xf00f, 0x8004);
    INSTRUCTION_DECLARE(SUB,        0xf00f, 0x8005);
    INSTRUCTION_DECLARE(SHR,        0xf00f, 0x8006);
    INSTRUCTION_DECLARE(SUBN,       0xf00f, 0x8007);
    INSTRUCTION_DECLARE(SHL,        0xf00f, 0x800e);
    INSTRUCTION_DECLARE(SNE_V,      0xf00f, 0x9000);
    INSTRUCTION_DECLARE(LD_I,       0xf000, 0xa000);
    INSTRUCTION_DECLARE(JP_V0_Addr, 0xf000, 0xb000);
    INSTRUCTION_DECLARE(RND,        0xf000, 0xc000);
    INSTRUCTION_DECLARE(DRW,        0xf000, 0xd000);
    INSTRUCTION_DECLARE(SKP,        0xf0ff, 0xe09e);
    INSTRUCTION_DECLARE(SKNP,       0xf0ff, 0xe0a1);
    INSTRUCTION_DECLARE(LD_V_DT,    0xf0ff, 0xf007);
    INSTRUCTION_DECLARE(LD_DT_V,    0xf0ff, 0xf015);
    INSTRUCTION_DECLARE(ADD_I_V,    0xf0ff, 0xf01e);

    #undef INSTRUCTION_DECLARE

}
