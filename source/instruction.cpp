#include "common.hpp"
#include "instruction.hpp"
#include "util.hpp"
#include "digits.hpp"
#include "chip8.hpp"
#include "assemble.hpp"

namespace tsh {

    #define INSTRUCTION_DISASSEMBLE(name) \
        DisassembleOutputIterator name::Disassemble(const DisassembleOutputIterator out, const Opcode op)

    #define INSTRUCTION_EXECUTE(name) \
        PCAdvance name::Execute(Chip8 &ch8, const Opcode op)

    #define INSTRUCTION_ASSEMBLE(name) \
        std::optional<Opcode> name::Assemble([[maybe_unused]] const Assembler &asmbl, const std::string_view ins)

    #define MATCH(pattern) ({                                       \
        const auto captures = util::WildcardCapture(pattern, ins);  \
        if (!captures.has_value()) {                                \
            return {};                                              \
        }                                                           \
        *captures;                                                  \
    })

    #define NOT_MATCH(pattern) if (util::WildcardCapture(pattern, ins).has_value()) return {}

    #define MUST_EXIST(expr) ({    \
        auto _value = (expr);      \
        if (!_value.has_value()) { \
            return {};             \
        }                          \
        *_value;                   \
    })

    INSTRUCTION_DISASSEMBLE(CLS) {
        UNUSED(op);

        return fmt::format_to(out, "CLS");
    }

    INSTRUCTION_EXECUTE(CLS) {
        UNUSED(op);

        ch8.display.Clear();

        return 1;
    }

    INSTRUCTION_ASSEMBLE(CLS) {
        MATCH("CLS");

        return Opcode(0x00E0);
    }

    INSTRUCTION_DISASSEMBLE(RET) {
        UNUSED(op);

        return fmt::format_to(out, "RET");
    }

    INSTRUCTION_EXECUTE(RET) {
        UNUSED(op);

        ch8.PC.Set(ch8.stack.top());
        ch8.stack.pop();

        return 1;
    }

    INSTRUCTION_ASSEMBLE(RET) {
        MATCH("RET");

        return Opcode(0x00EE);
    }

    INSTRUCTION_DISASSEMBLE(JP_Addr) {
        return fmt::format_to(out, "JP 0x{:03X}", op.Addr());
    }

    INSTRUCTION_EXECUTE(JP_Addr) {
        ch8.PC.Set(op.Addr());

        return 0;
    }

    INSTRUCTION_ASSEMBLE(JP_Addr) {
        NOT_MATCH("JP V0, *");

        const auto captures = MATCH("JP *");
        const auto addr     = MUST_EXIST(asmbl.ToAddress(captures[0]));

        return Opcode()
            .TopNibble(0x1)
            .Addr(addr);
    }

    INSTRUCTION_DISASSEMBLE(CALL) {
        return fmt::format_to(out, "CALL 0x{:03X}", op.Addr());
    }

    INSTRUCTION_EXECUTE(CALL) {
        ch8.stack.push(ch8.PC.Get());
        ch8.PC.Set(op.Addr());

        return 0;
    }

    INSTRUCTION_ASSEMBLE(CALL) {
        const auto captures = MATCH("CALL *");
        const auto addr     = MUST_EXIST(asmbl.ToAddress(captures[0]));

        return Opcode()
            .TopNibble(0x2)
            .Addr(addr);
    }

    INSTRUCTION_DISASSEMBLE(SE_V_Byte) {
        return fmt::format_to(out, "SE V{:01X}, 0x{:02X}", op.X(), op.Byte());
    }

    INSTRUCTION_EXECUTE(SE_V_Byte) {
        if (ch8.V[op.X()].Get() == op.Byte()) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_ASSEMBLE(SE_V_Byte) {
        NOT_MATCH("SE V*, V*");

        const auto captures = MATCH("SE V*, *");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));
        const auto byte     = MUST_EXIST(Assembler::ToNumber<std::uint8_t>(captures[1]));

        return Opcode()
            .TopNibble(0x3)
            .X(reg)
            .Byte(byte);
    }

    INSTRUCTION_DISASSEMBLE(SNE_V_Byte) {
        return fmt::format_to(out, "SNE V{:01X}, 0x{:02X}", op.X(), op.Byte());
    }

    INSTRUCTION_EXECUTE(SNE_V_Byte) {
        if (ch8.V[op.X()].Get() != op.Byte()) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_ASSEMBLE(SNE_V_Byte) {
        NOT_MATCH("SNE V* V*");

        const auto captures = MATCH("SNE V*, *");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));
        const auto byte     = MUST_EXIST(Assembler::ToNumber<std::uint8_t>(captures[1]));

        return Opcode()
            .TopNibble(0x4)
            .X(reg)
            .Byte(byte);
    }

    INSTRUCTION_DISASSEMBLE(SE_V_V) {
        return fmt::format_to(out, "SE V{:01X}, V{:01X}", op.X(), op.Y());
    }

    INSTRUCTION_EXECUTE(SE_V_V) {
        if (ch8.V[op.X()].Get() == ch8.V[op.Y()].Get()) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_ASSEMBLE(SE_V_V) {
        const auto captures = MATCH("SE V*, V*");
        const auto reg_x    = MUST_EXIST(Assembler::RegisterNibble(captures[0]));
        const auto reg_y    = MUST_EXIST(Assembler::RegisterNibble(captures[1]));

        return Opcode()
            .TopNibble(0x5)
            .X(reg_x)
            .Y(reg_y)
            .Nibble(0x0);
    }

    INSTRUCTION_DISASSEMBLE(LD_V_Byte) {
        return fmt::format_to(out, "LD V{:01X}, 0x{:02X}", op.X(), op.Byte());
    }

    INSTRUCTION_EXECUTE(LD_V_Byte) {
        ch8.V[op.X()].Set(op.Byte());

        return 1;
    }

    INSTRUCTION_ASSEMBLE(LD_V_Byte) {
        NOT_MATCH("LD V*, V*");
        NOT_MATCH("LD V*, DT");
        NOT_MATCH("LD V*, K");
        NOT_MATCH("LD V*, [I]");

        const auto captures = MATCH("LD V*, *");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));
        const auto byte     = MUST_EXIST(Assembler::ToNumber<std::uint8_t>(captures[1]));

        return Opcode()
            .TopNibble(0x6)
            .X(reg)
            .Byte(byte);
    }

    INSTRUCTION_DISASSEMBLE(ADD_V_Byte) {
        return fmt::format_to(out, "ADD V{:01X}, 0x{:02X}", op.X(), op.Byte());
    }

    INSTRUCTION_EXECUTE(ADD_V_Byte) {
        ch8.V[op.X()].Increment(op.Byte());

        return 1;
    }

    INSTRUCTION_ASSEMBLE(ADD_V_Byte) {
        NOT_MATCH("ADD V*, V*");

        const auto captures = MATCH("ADD V*, *");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));
        const auto byte     = MUST_EXIST(Assembler::ToNumber<std::uint8_t>(captures[1]));

        return Opcode()
            .TopNibble(0x7)
            .X(reg)
            .Byte(byte);
    }

    INSTRUCTION_DISASSEMBLE(LD_V_V) {
        return fmt::format_to(out, "LD V{:01X}, V{:01X}", op.X(), op.Y());
    }

    INSTRUCTION_EXECUTE(LD_V_V) {
        ch8.V[op.X()].Set(ch8.V[op.Y()].Get());

        return 1;
    }

    INSTRUCTION_ASSEMBLE(LD_V_V) {
        const auto captures = MATCH("LD V*, V*");
        const auto reg_x    = MUST_EXIST(Assembler::RegisterNibble(captures[0]));
        const auto reg_y    = MUST_EXIST(Assembler::RegisterNibble(captures[1]));

        return Opcode()
            .TopNibble(0x8)
            .X(reg_x)
            .Y(reg_y)
            .Nibble(0x0);
    }

    INSTRUCTION_DISASSEMBLE(OR_V_V) {
        return fmt::format_to(out, "OR V{:01X}, V{:01X}", op.X(), op.Y());
    }

    INSTRUCTION_EXECUTE(OR_V_V) {
        const auto &x = ch8.V[op.X()].Get();
        const auto &y = ch8.V[op.Y()].Get();

        ch8.V[op.X()].Set(x | y);

        return 1;
    }

    INSTRUCTION_ASSEMBLE(OR_V_V) {
        const auto captures = MATCH("OR V*, V*");
        const auto reg_x    = MUST_EXIST(Assembler::RegisterNibble(captures[0]));
        const auto reg_y    = MUST_EXIST(Assembler::RegisterNibble(captures[1]));

        return Opcode()
            .TopNibble(0x8)
            .X(reg_x)
            .Y(reg_y)
            .Nibble(0x1);
    }

    INSTRUCTION_DISASSEMBLE(AND_V_V) {
        return fmt::format_to(out, "AND V{:01X}, V{:01X}", op.X(), op.Y());
    }

    INSTRUCTION_EXECUTE(AND_V_V) {
        const auto &x = ch8.V[op.X()].Get();
        const auto &y = ch8.V[op.Y()].Get();

        ch8.V[op.X()].Set(x & y);

        return 1;
    }

    INSTRUCTION_ASSEMBLE(AND_V_V) {
        const auto captures = MATCH("AND V*, V*");
        const auto reg_x    = MUST_EXIST(Assembler::RegisterNibble(captures[0]));
        const auto reg_y    = MUST_EXIST(Assembler::RegisterNibble(captures[1]));

        return Opcode()
            .TopNibble(0x8)
            .X(reg_x)
            .Y(reg_y)
            .Nibble(0x2);
    }

    INSTRUCTION_DISASSEMBLE(XOR_V_V) {
        return fmt::format_to(out, "XOR V{:01X}, V{:01X}", op.X(), op.Y());
    }

    INSTRUCTION_EXECUTE(XOR_V_V) {
        const auto &x = ch8.V[op.X()].Get();
        const auto &y = ch8.V[op.Y()].Get();

        ch8.V[op.X()].Set(x ^ y);

        return 1;
    }

    INSTRUCTION_ASSEMBLE(XOR_V_V) {
        const auto captures = MATCH("XOR V*, V*");
        const auto reg_x    = MUST_EXIST(Assembler::RegisterNibble(captures[0]));
        const auto reg_y    = MUST_EXIST(Assembler::RegisterNibble(captures[1]));

        return Opcode()
            .TopNibble(0x8)
            .X(reg_x)
            .Y(reg_y)
            .Nibble(0x3);
    }

    INSTRUCTION_DISASSEMBLE(ADD_V_V) {
        return fmt::format_to(out, "ADD V{:01X}, V{:01X}", op.X(), op.Y());
    }

    INSTRUCTION_EXECUTE(ADD_V_V) {
              auto &reg_x = ch8.V[op.X()];
        const auto &reg_y = ch8.V[op.Y()];

        const auto x = static_cast<std::uint16_t>(reg_x.Get());
        const auto y = static_cast<std::uint16_t>(reg_y.Get());

        const auto result = x + y;
        if (result > reg_x.Max()) {
            ch8.V[0xF].Set(1);
        } else {
            ch8.V[0xF].Set(0);
        }

        reg_x.Set(static_cast<std::uint8_t>(result & 0xFF));

        return 1;
    }

    INSTRUCTION_ASSEMBLE(ADD_V_V) {
        const auto captures = MATCH("ADD V*, V*");
        const auto reg_x    = MUST_EXIST(Assembler::RegisterNibble(captures[0]));
        const auto reg_y    = MUST_EXIST(Assembler::RegisterNibble(captures[1]));

        return Opcode()
            .TopNibble(0x8)
            .X(reg_x)
            .Y(reg_y)
            .Nibble(0x4);
    }

    INSTRUCTION_DISASSEMBLE(SUB_V_V) {
        return fmt::format_to(out, "SUB V{:01X}, V{:01X}", op.X(), op.Y());
    }

    INSTRUCTION_EXECUTE(SUB_V_V) {
        const auto &x = ch8.V[op.X()].Get();
        const auto &y = ch8.V[op.Y()].Get();

        if (x > y) {
            ch8.V[0xF].Set(1);
        } else {
            ch8.V[0xF].Set(0);
        }

        ch8.V[op.X()].Set(x - y);

        return 1;
    }

    INSTRUCTION_ASSEMBLE(SUB_V_V) {
        const auto captures = MATCH("SUB V*, V*");
        const auto reg_x    = MUST_EXIST(Assembler::RegisterNibble(captures[0]));
        const auto reg_y    = MUST_EXIST(Assembler::RegisterNibble(captures[1]));

        return Opcode()
            .TopNibble(0x8)
            .X(reg_x)
            .Y(reg_y)
            .Nibble(0x5);
    }

    INSTRUCTION_DISASSEMBLE(SHR_V) {
        return fmt::format_to(out, "SHR V{:01X}", op.X());
    }

    INSTRUCTION_EXECUTE(SHR_V) {
              auto &reg_x = ch8.V[op.X()];
        const auto &x     = reg_x.Get();

        if (reg_x.IsBitSet(0)) {
            ch8.V[0xF].Set(1);
        } else {
            ch8.V[0xF].Set(0);
        }

        reg_x.Set(x >> 1);

        return 1;
    }

    INSTRUCTION_ASSEMBLE(SHR_V) {
        const auto captures = MATCH("SHR V*");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));

        return Opcode()
            .TopNibble(0x8)
            .X(reg)
            .Y(0x0)
            .Nibble(0x6);
    }

    INSTRUCTION_DISASSEMBLE(SUBN_V_V) {
        return fmt::format_to(out, "SUBN V{:01X}, V{:01X}", op.X(), op.Y());
    }

    INSTRUCTION_EXECUTE(SUBN_V_V) {
        const auto &x = ch8.V[op.X()].Get();
        const auto &y = ch8.V[op.Y()].Get();

        if (y > x) {
            ch8.V[0xF].Set(1);
        } else {
            ch8.V[0xF].Set(0);
        }

        ch8.V[op.X()].Set(y - x);

        return 1;
    }

    INSTRUCTION_ASSEMBLE(SUBN_V_V) {
        const auto captures = MATCH("SUBN V*, V*");
        const auto reg_x    = MUST_EXIST(Assembler::RegisterNibble(captures[0]));
        const auto reg_y    = MUST_EXIST(Assembler::RegisterNibble(captures[1]));

        return Opcode()
            .TopNibble(0x8)
            .X(reg_x)
            .Y(reg_y)
            .Nibble(0x7);
    }

    INSTRUCTION_DISASSEMBLE(SHL_V) {
        return fmt::format_to(out, "SHL V{:01X}", op.X());
    }

    INSTRUCTION_EXECUTE(SHL_V) {
              auto &reg_x = ch8.V[op.X()];
        const auto &x     = reg_x.Get();

        if (reg_x.IsBitSet(7)) {
            ch8.V[0xF].Set(1);
        } else {
            ch8.V[0xF].Set(0);
        }

        reg_x.Set(x << 1);

        return 1;
    }

    INSTRUCTION_ASSEMBLE(SHL_V) {
        const auto captures = MATCH("SHL V*");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));

        return Opcode()
            .TopNibble(0x8)
            .X(reg)
            .Y(0x0)
            .Nibble(0xE);
    }

    INSTRUCTION_DISASSEMBLE(SNE_V_V) {
        return fmt::format_to(out, "SNE V{:01X}, V{:01X}", op.X(), op.Y());
    }

    INSTRUCTION_EXECUTE(SNE_V_V) {
        if (ch8.V[op.X()].Get() != ch8.V[op.Y()].Get()) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_ASSEMBLE(SNE_V_V) {
        const auto captures = MATCH("SNE V*, V*");
        const auto reg_x    = MUST_EXIST(Assembler::RegisterNibble(captures[0]));
        const auto reg_y    = MUST_EXIST(Assembler::RegisterNibble(captures[1]));

        return Opcode()
            .TopNibble(0x9)
            .X(reg_x)
            .Y(reg_y)
            .Nibble(0x0);
    }

    INSTRUCTION_DISASSEMBLE(LD_I_Addr) {
        return fmt::format_to(out, "LD I, 0x{:03X}", op.Addr());
    }

    INSTRUCTION_EXECUTE(LD_I_Addr) {
        ch8.I.Set(op.Addr());

        return 1;
    }

    INSTRUCTION_ASSEMBLE(LD_I_Addr) {
        const auto captures = MATCH("LD I, *");
        const auto addr     = MUST_EXIST(asmbl.ToAddress(captures[0]));

        return Opcode()
            .TopNibble(0xA)
            .Addr(addr);
    }

    INSTRUCTION_DISASSEMBLE(JP_V0_Addr) {
        return fmt::format_to(out, "JP V0, 0x{:03X}", op.Addr());
    }

    INSTRUCTION_EXECUTE(JP_V0_Addr) {
        ch8.PC.Set(op.Addr() + ch8.V[0x0].Get());

        return 0;
    }

    INSTRUCTION_ASSEMBLE(JP_V0_Addr) {
        const auto captures = MATCH("JP V0, *");
        const auto addr     = MUST_EXIST(asmbl.ToAddress(captures[0]));

        return Opcode()
            .TopNibble(0xB)
            .Addr(addr);
    }

    INSTRUCTION_DISASSEMBLE(RND) {
        return fmt::format_to(out, "RND V{:01X}, 0x{:02X}", op.X(), op.Byte());
    }

    INSTRUCTION_EXECUTE(RND) {
        ch8.V[op.X()].Set(ch8.rng.RandomU8() & op.Byte());

        return 1;
    }

    INSTRUCTION_ASSEMBLE(RND) {
        const auto captures = MATCH("RND V*, *");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));
        const auto byte     = MUST_EXIST(Assembler::ToNumber<std::uint8_t>(captures[1]));

        return Opcode()
            .TopNibble(0xC)
            .X(reg)
            .Byte(byte);
    }

    INSTRUCTION_DISASSEMBLE(DRW) {
        return fmt::format_to(out, "DRW V{:01X}, V{:01X}, 0x{:01X}", op.X(), op.Y(), op.Nibble());
    }

    INSTRUCTION_EXECUTE(DRW) {
        const auto sprite_data = std::span<const std::byte>(ch8.memory.data() + ch8.I.Get(), op.Nibble());

        const auto collide = ch8.display.DrawSprite(ch8.V[op.X()].Get(), ch8.V[op.Y()].Get(), sprite_data);
        if (collide) {
            ch8.V[0xF].Set(1);
        } else {
            ch8.V[0xF].Set(0);
        }

        return 1;
    }

    INSTRUCTION_ASSEMBLE(DRW) {
        const auto captures = MATCH("DRW V*, V*, *");
        const auto reg_x    = MUST_EXIST(Assembler::RegisterNibble(captures[0]));
        const auto reg_y    = MUST_EXIST(Assembler::RegisterNibble(captures[1]));
        const auto height   = MUST_EXIST(Assembler::ToNumber<std::uint8_t>(captures[2]));

        /* If height can't fit in a nibble. */
        if (height > 0xF) {
            return {};
        }

        return Opcode()
            .TopNibble(0xD)
            .X(reg_x)
            .Y(reg_y)
            .Nibble(height);
    }

    INSTRUCTION_DISASSEMBLE(SKP) {
        return fmt::format_to(out, "SKP V{:01X}", op.X());
    }

    INSTRUCTION_EXECUTE(SKP) {
        std::this_thread::sleep_for(Chip8::FrameDuration);

        const auto key = static_cast<Key>(ch8.V[op.X()].Get());

        if (ch8.keyboard.IsKeyPressed(key)) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_ASSEMBLE(SKP) {
        const auto captures = MATCH("SKP V*");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));

        return Opcode()
            .TopNibble(0xE)
            .X(reg)
            .Byte(0x9E);
    }

    INSTRUCTION_DISASSEMBLE(SKNP) {
        return fmt::format_to(out, "SKNP V{:01X}", op.X());
    }

    INSTRUCTION_EXECUTE(SKNP) {
        std::this_thread::sleep_for(Chip8::FrameDuration);

        const auto key = static_cast<Key>(ch8.V[op.X()].Get());

        if (!ch8.keyboard.IsKeyPressed(key)) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_ASSEMBLE(SKNP) {
        const auto captures = MATCH("SKNP V*");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));

        return Opcode()
            .TopNibble(0xE)
            .X(reg)
            .Byte(0xA1);
    }

    INSTRUCTION_DISASSEMBLE(LD_V_DT) {
        return fmt::format_to(out, "LD V{:01X}, DT", op.X());
    }

    INSTRUCTION_EXECUTE(LD_V_DT) {
        ch8.V[op.X()].Set(ch8.DT.Get());

        return 1;
    }

    INSTRUCTION_ASSEMBLE(LD_V_DT) {
        const auto captures = MATCH("LD V*, DT");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));

        return Opcode()
            .TopNibble(0xF)
            .X(reg)
            .Byte(0x07);
    }

    INSTRUCTION_DISASSEMBLE(LD_V_K) {
        return fmt::format_to(out, "LD V{:01X}, K", op.X());
    }

    INSTRUCTION_EXECUTE(LD_V_K) {
        const auto key = ch8.keyboard.CurrentKey();

        if (key == Key::Invalid) {
            return 0;
        }

        ch8.V[op.X()].Set(static_cast<std::uint8_t>(key));

        return 1;
    }

    INSTRUCTION_ASSEMBLE(LD_V_K) {
        const auto captures = MATCH("LD V*, K");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));

        return Opcode()
            .TopNibble(0xF)
            .X(reg)
            .Byte(0x0A);
    }

    INSTRUCTION_DISASSEMBLE(LD_DT_V) {
        return fmt::format_to(out, "LD DT, V{:01X}", op.X());
    }

    INSTRUCTION_EXECUTE(LD_DT_V) {
        ch8.DT.Set(ch8.V[op.X()].Get());

        return 1;
    }

    INSTRUCTION_ASSEMBLE(LD_DT_V) {
        const auto captures = MATCH("LD DT, V*");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));

        return Opcode()
            .TopNibble(0xF)
            .X(reg)
            .Byte(0x15);
    }

    INSTRUCTION_DISASSEMBLE(LD_ST_V) {
        return fmt::format_to(out, "LD ST, V{:01X}", op.X());
    }

    INSTRUCTION_EXECUTE(LD_ST_V) {
        ch8.ST.Set(ch8.V[op.X()].Get());

        return 1;
    }

    INSTRUCTION_ASSEMBLE(LD_ST_V) {
        const auto captures = MATCH("LD ST, V*");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));

        return Opcode()
            .TopNibble(0xF)
            .X(reg)
            .Byte(0x18);
    }

    INSTRUCTION_DISASSEMBLE(ADD_I_V) {
        return fmt::format_to(out, "ADD I, V{:01X}", op.X());
    }

    INSTRUCTION_EXECUTE(ADD_I_V) {
        ch8.I.Increment(ch8.V[op.X()].Get());

        return 1;
    }

    INSTRUCTION_ASSEMBLE(ADD_I_V) {
        const auto captures = MATCH("ADD I, V*");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));

        return Opcode()
            .TopNibble(0xF)
            .X(reg)
            .Byte(0x1E);
    }

    INSTRUCTION_DISASSEMBLE(LD_F_V) {
        return fmt::format_to(out, "LD F, V{:01X}", op.X());
    }

    INSTRUCTION_EXECUTE(LD_F_V) {
        ch8.I.Set(Chip8::DigitSpace.start + sizeof(Digit) * ch8.V[op.X()].Get());

        return 1;
    }

    INSTRUCTION_ASSEMBLE(LD_F_V) {
        const auto captures = MATCH("LD F, V*");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));

        return Opcode()
            .TopNibble(0xF)
            .X(reg)
            .Byte(0x29);
    }

    INSTRUCTION_DISASSEMBLE(LD_B_V) {
        return fmt::format_to(out, "LD B, V{:01X}", op.X());
    }

    INSTRUCTION_EXECUTE(LD_B_V) {
        static constexpr std::uint8_t MaxPower = 2;

        const auto &addr = ch8.I.Get();

        auto num = ch8.V[op.X()].Get();
        for (const auto offset : std::views::iota(0, MaxPower + 1) | std::views::reverse) {
            const auto digit = num % 10;

            ch8.memory[addr + offset] = static_cast<std::byte>(digit);

            num /= 10;
        }

        return 1;
    }

    INSTRUCTION_ASSEMBLE(LD_B_V) {
        const auto captures = MATCH("LD B, V*");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));

        return Opcode()
            .TopNibble(0xF)
            .X(reg)
            .Byte(0x33);
    }

    INSTRUCTION_DISASSEMBLE(LD_DEREF_I_V) {
        return fmt::format_to(out, "LD [I], V{:01X}", op.X());
    }

    INSTRUCTION_EXECUTE(LD_DEREF_I_V) {
        const auto &addr = ch8.I.Get();

        for (const auto offset : std::views::iota(0, op.X() + 1)) {
            const auto &reg = ch8.V[offset];

            ch8.memory[addr + offset] = static_cast<std::byte>(reg.Get());
        }

        return 1;
    }

    INSTRUCTION_ASSEMBLE(LD_DEREF_I_V) {
        const auto captures = MATCH("LD [I], V*");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));

        return Opcode()
            .TopNibble(0xF)
            .X(reg)
            .Byte(0x55);
    }

    INSTRUCTION_DISASSEMBLE(LD_V_DEREF_I) {
        return fmt::format_to(out, "LD V{:01X}, [I]", op.X());
    }

    INSTRUCTION_EXECUTE(LD_V_DEREF_I) {
        const auto &addr = ch8.I.Get();

        for (const auto offset : std::views::iota(0, op.X() + 1)) {
            auto &reg = ch8.V[offset];

            reg.Set(static_cast<std::uint8_t>(ch8.memory[addr + offset]));
        }

        return 1;
    }

    INSTRUCTION_ASSEMBLE(LD_V_DEREF_I) {
        const auto captures = MATCH("LD V*, [I]");
        const auto reg      = MUST_EXIST(Assembler::RegisterNibble(captures[0]));

        return Opcode()
            .TopNibble(0xF)
            .X(reg)
            .Byte(0x65);
    }

    #undef INSTRUCTION_ASSEMBLE
    #undef INSTRUCTION_EXECUTE
    #undef INSTRUCTION_DISASSEMBLE

    #define ASM_ONLY_INSTRUCTION_ASSEMBLE(name) \
        std::optional<std::uint8_t> name::Assemble([[maybe_unused]] const Assembler &asmbl, const std::string_view ins)

    ASM_ONLY_INSTRUCTION_ASSEMBLE(BYTE) {
        const auto captures = MATCH(".byte *");
        const auto byte     = MUST_EXIST(Assembler::ToNumber<std::uint8_t>(captures[0]));

        return byte;
    }

    ASM_ONLY_INSTRUCTION_ASSEMBLE(SPRITE) {
        static constexpr auto Padding = Digit::Padding;

        const auto captures   = MATCH(".sprite \"*\"");
              auto sprite_str = captures[0];

        if (sprite_str.size() > BITSIZEOF(std::byte) + 2 * Padding) {
            return {};
        }

        sprite_str = sprite_str.substr(Padding, sprite_str.size() - 2 * Padding);

        std::uint8_t sprite_row = {};
        for (const auto &&[i, c] : util::enumerate(sprite_str)) {
            if (c == '*') {
                sprite_row |= (std::uint8_t{1} << i);
            } else if (c == ' ') {
                sprite_row |= (std::uint8_t{0} << i);
            } else {
                return {};
            }
        }

        sprite_row <<= BITSIZEOF(std::byte) - sprite_str.size();

        return sprite_row;
    }

    #undef MUST_EXIST
    #undef NOT_MATCH
    #undef MATCH

}
