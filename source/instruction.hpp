#pragma once

#include "common.hpp"
#include "nibble_pattern.hpp"

namespace tsh {

    /* Forward declare. */
    class Chip8;

    /* Forward declare. */
    class Assembler;

    class Opcode {
        public:
            RawOpcode op = {};

            ALWAYS_INLINE constexpr Opcode() = default;
            ALWAYS_INLINE constexpr explicit Opcode(const RawOpcode op) : op(op) { }

            [[nodiscard]]
            ALWAYS_INLINE constexpr const RawOpcode &Get() const {
                return this->op;
            }

            ALWAYS_INLINE constexpr Opcode &Set(const RawOpcode value) {
                this->op = value;

                return *this;
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr Address Addr() const {
                return this->Get() & 0x0FFF;
            }

            ALWAYS_INLINE constexpr Opcode &Addr(const Address addr) {
                this->Set((this->Get() & 0xF000) | addr);

                return *this;
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr std::uint8_t X() const {
                return (this->Get() & 0x0F00) >> 8;
            }

            ALWAYS_INLINE constexpr Opcode &X(const std::uint8_t x) {
                this->Set((this->Get() & 0xF0FF) | (static_cast<std::uint16_t>(x) << 8));

                return *this;
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr std::uint8_t Y() const {
                return (this->Get() & 0x00F0) >> 4;
            }

            ALWAYS_INLINE constexpr Opcode &Y(const std::uint8_t y) {
                this->Set((this->Get() & 0xFF0F) | (y << 4));

                return *this;
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr std::uint8_t Byte() const {
                return this->Get() & 0x00FF;
            }

            ALWAYS_INLINE constexpr Opcode &Byte(const std::uint8_t byte) {
                this->Set((this->Get() & 0xFF00) | byte);

                return *this;
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr std::uint8_t Nibble() const {
                return this->Get() & 0x000F;
            }

            ALWAYS_INLINE constexpr Opcode &Nibble(const std::uint8_t nibble) {
                this->Set((this->Get() & 0xFFF0) | nibble);

                return *this;
            }

            [[nodiscard]]
            ALWAYS_INLINE constexpr std::uint8_t TopNibble() const {
                return (this->Get() & 0xF000) >> 12;
            }

            ALWAYS_INLINE constexpr Opcode &TopNibble(const std::uint8_t nibble) {
                this->Set((this->Get() & 0x0FFF) | (nibble << 12));

                return *this;
            }
    };

    static_assert(sizeof(Opcode) == sizeof(RawOpcode));
    static_assert(
        Opcode()
            .TopNibble(0xC)
            .X(0xA)
            .Y(0xF)
            .Nibble(0xE)
            .Get() == 0xCAFE
    );

    using PCAdvance = std::int32_t;

    template<typename T>
    concept AssemblyData = std::same_as<T, std::optional<std::uint8_t>> || std::same_as<T, std::optional<Opcode>>;

    template<typename T>
    concept Instruction = requires(Chip8 &ch8, Opcode op, DisassembleOutputIterator out, const Assembler &asmbl, const std::string_view ins) {
        { T::Compare(op) }          -> std::same_as<bool>;
        { T::Execute(ch8, op) }     -> std::same_as<PCAdvance>;
        { T::Disassemble(out, op) } -> std::same_as<DisassembleOutputIterator>;
        { T::Assemble(asmbl, ins) } -> AssemblyData;
    };

    template<typename T>
    concept SpecialSizedInstruction = Instruction<T> && requires {
        T::NameWithTrailingSpace;
        T::Size;
    } && std::same_as<std::remove_cvref_t<decltype(T::NameWithTrailingSpace)>, std::string_view> &&
         std::same_as<std::remove_cvref_t<decltype(T::Size)>,                  Address>;


    template<Instruction Ins, typename... Ts>
    class InstructionHandler {
        public:
            [[nodiscard]]
            static std::optional<PCAdvance> Execute(Chip8 &ch8, const Opcode op) {
                if (Ins::Compare(op)) {
                    return Ins::Execute(ch8, op);
                }

                return InstructionHandler<Ts...>::Execute(ch8, op);
            }

            [[nodiscard]]
            static std::optional<DisassembleOutputIterator> Disassemble(DisassembleOutputIterator out, const Address address, const Opcode op) {
                if (Ins::Compare(op)) {
                    out = fmt::format_to(out, "{:04X}: ({:04X}) -> ", address, op.Get());
                    out = Ins::Disassemble(out, op);

                    return fmt::format_to(out, "\n");
                }

                return InstructionHandler<Ts...>::Disassemble(out, address, op);
            }

            [[nodiscard]]
            static std::optional<std::vector<std::byte>> Assemble(const Assembler &asmbl, const std::string_view ins) {
                const auto data = Ins::Assemble(asmbl, ins);
                if (data.has_value()) {
                    if constexpr (std::same_as<std::remove_cvref_t<decltype(*data)>, Opcode>) {
                        return std::vector{
                            static_cast<std::byte>(data->Get() >> 8),
                            static_cast<std::byte>(data->Get() & 0xFF),
                        };
                    } else {
                        /* Only other option is single uint8_t. */

                        return std::vector{static_cast<std::byte>(*data)};
                    }
                }

                return InstructionHandler<Ts...>::Assemble(asmbl, ins);
            }

            [[nodiscard]]
            static constexpr Address Size(const std::string_view ins) {
                if constexpr (SpecialSizedInstruction<Ins>) {
                    if (Ins::NameWithTrailingSpace == ins.substr(0, Ins::NameWithTrailingSpace.size())) {
                        return Ins::Size;
                    }
                }

                return InstructionHandler<Ts...>::Size(ins);
            }
    };

    /* Base case. */
    template<Instruction Ins>
    class InstructionHandler<Ins> {
        public:
            [[nodiscard]]
            static std::optional<PCAdvance> Execute(Chip8 &ch8, const Opcode op) {
                if (Ins::Compare(op)) {
                    return Ins::Execute(ch8, op);
                }

                return {};
            }

            [[nodiscard]]
            static std::optional<DisassembleOutputIterator> Disassemble(DisassembleOutputIterator out, const Address address, const Opcode op) {
                if (Ins::Compare(op)) {
                    out = fmt::format_to(out, "{:04X}: ({:04X}) -> ", address, op.Get());
                    out = Ins::Disassemble(out, op);

                    return fmt::format_to(out, "\n");
                }

                return {};
            }

            [[nodiscard]]
            static std::optional<std::vector<std::byte>> Assemble(const Assembler &asmbl, const std::string_view ins) {
                const auto data = Ins::Assemble(asmbl, ins);
                if (data.has_value()) {
                    if constexpr (std::same_as<std::remove_cvref_t<decltype(*data)>, Opcode>) {
                        return std::vector{
                            static_cast<std::byte>(data->Get() >> 8),
                            static_cast<std::byte>(data->Get() & 0xFF),
                        };
                    } else {
                        /* Only other option is single uint8_t. */

                        return std::vector{static_cast<std::byte>(*data)};
                    }
                }

                return {};
            }

            [[nodiscard]]
            static constexpr Address Size(const std::string_view ins) {
                if constexpr (SpecialSizedInstruction<Ins>) {
                    if (Ins::NameWithTrailingSpace == ins.substr(0, Ins::NameWithTrailingSpace.size())) {
                        return Ins::Size;
                    }
                }

                return sizeof(Opcode);
            }
    };

    #define INSTRUCTION_DECLARE(name, pattern)                                                                      \
        class name {                                                                                                \
            public:                                                                                                 \
                static constexpr auto Pattern = NibblePattern(pattern);                                             \
                [[nodiscard]]                                                                                       \
                ALWAYS_INLINE static constexpr bool Compare(const Opcode op) {                                      \
                    return Pattern.matches(op.Get());                                                               \
                }                                                                                                   \
                [[nodiscard]]                                                                                       \
                static PCAdvance Execute(Chip8 &ch8, const Opcode op);                                              \
                [[nodiscard]]                                                                                       \
                static DisassembleOutputIterator Disassemble(const DisassembleOutputIterator out, const Opcode op); \
                [[nodiscard]]                                                                                       \
                static std::optional<Opcode> Assemble(const Assembler &asmbl, const std::string_view ins);          \
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

    #define ASM_ONLY_INSTRUCTION_DECLARE(name, size, ins_name)                                                                     \
        class name {                                                                                                               \
            public:                                                                                                                \
                static constexpr std::string_view NameWithTrailingSpace = ins_name " ";                                            \
                static constexpr Address          Size                  = size;                                                    \
                [[nodiscard]]                                                                                                      \
                ALWAYS_INLINE static constexpr bool Compare(const Opcode op) {                                                     \
                    UNUSED(op);                                                                                                    \
                    return false;                                                                                                  \
                }                                                                                                                  \
                [[nodiscard]]                                                                                                      \
                ALWAYS_INLINE static PCAdvance Execute(Chip8 &ch8, const Opcode op) {                                              \
                    UNUSED(ch8, op);                                                                                               \
                    return 0;                                                                                                      \
                }                                                                                                                  \
                [[nodiscard]]                                                                                                      \
                ALWAYS_INLINE static DisassembleOutputIterator Disassemble(const DisassembleOutputIterator out, const Opcode op) { \
                    UNUSED(op);                                                                                                    \
                    return out;                                                                                                    \
                }                                                                                                                  \
                [[nodiscard]]                                                                                                      \
                static std::optional<std::uint8_t> Assemble(const Assembler &asmbl, const std::string_view ins);                   \
        }

    ASM_ONLY_INSTRUCTION_DECLARE(SPRITE, 1, ".sprite");
    ASM_ONLY_INSTRUCTION_DECLARE(BYTE,   1, ".byte");

    #undef ASM_ONLY_INSTRUCTION_DECLARE

}
