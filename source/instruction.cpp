#include "common.hpp"
#include "instruction.hpp"
#include "chip8.hpp"

namespace tsh {

    #define INSTRUCTION_EXECUTE(name) \
        PCAdvance name::Execute(Chip8 &ch8, const Opcode op)

    INSTRUCTION_EXECUTE(CLS) {
        UNUSED(op);

        ch8.display.Clear();

        return 1;
    }

    INSTRUCTION_EXECUTE(RET) {
        UNUSED(op);

        ch8.PC.Set(ch8.stack.top());
        ch8.stack.pop();

        return 1;
    }

    INSTRUCTION_EXECUTE(JP_Addr) {
        ch8.PC.Set(op.Addr());

        return 0;
    }

    INSTRUCTION_EXECUTE(CALL) {
        ch8.stack.push(ch8.PC.Get());
        ch8.PC.Set(op.Addr());

        return 0;
    }

    INSTRUCTION_EXECUTE(SE_V_Byte) {
        if (ch8.V[op.X()].Get() == op.Byte()) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_EXECUTE(SNE_V_Byte) {
        if (ch8.V[op.X()].Get() != op.Byte()) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_EXECUTE(SE_V_V) {
        if (ch8.V[op.X()].Get() == ch8.V[op.Y()].Get()) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_EXECUTE(LD_V_Byte) {
        ch8.V[op.X()].Set(op.Byte());

        return 1;
    }

    INSTRUCTION_EXECUTE(ADD_V_Byte) {
        ch8.V[op.X()].Increment(op.Byte());

        return 1;
    }

    INSTRUCTION_EXECUTE(LD_V_V) {
        ch8.V[op.X()].Set(ch8.V[op.Y()].Get());

        return 1;
    }

    INSTRUCTION_EXECUTE(OR_V_V) {
        const auto &x = ch8.V[op.X()].Get();
        const auto &y = ch8.V[op.Y()].Get();

        ch8.V[op.X()].Set(x | y);

        return 1;
    }

    INSTRUCTION_EXECUTE(AND_V_V) {
        const auto &x = ch8.V[op.X()].Get();
        const auto &y = ch8.V[op.Y()].Get();

        ch8.V[op.X()].Set(x & y);

        return 1;
    }

    INSTRUCTION_EXECUTE(XOR_V_V) {
        const auto &x = ch8.V[op.X()].Get();
        const auto &y = ch8.V[op.Y()].Get();

        ch8.V[op.X()].Set(x ^ y);

        return 1;
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

    INSTRUCTION_EXECUTE(SNE_V_V) {
        if (ch8.V[op.X()].Get() != ch8.V[op.Y()].Get()) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_EXECUTE(LD_I_Addr) {
        ch8.I.Set(op.Addr());

        return 1;
    }

    INSTRUCTION_EXECUTE(JP_V0_Addr) {
        ch8.PC.Set(op.Addr() + ch8.V[0x0].Get());

        return 0;
    }

    INSTRUCTION_EXECUTE(RND) {
        ch8.V[op.X()].Set(ch8.rng.RandomU8() & op.Byte());

        return 1;
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

    INSTRUCTION_EXECUTE(SKP) {
        std::this_thread::sleep_for(Chip8::FrameDuration);

        const auto key = static_cast<Key>(op.X());

        if (ch8.keyboard.IsKeyPressed(key)) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_EXECUTE(SKNP) {
        std::this_thread::sleep_for(Chip8::FrameDuration);

        const auto key = static_cast<Key>(op.X());

        if (!ch8.keyboard.IsKeyPressed(key)) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_EXECUTE(LD_V_DT) {
        ch8.V[op.X()].Set(ch8.DT.Get());

        return 1;
    }

    INSTRUCTION_EXECUTE(LD_V_K) {
        const auto key = ch8.keyboard.CurrentKey();

        if (key == Key::Invalid) {
            return 0;
        }

        ch8.V[op.X()].Set(static_cast<std::uint8_t>(key));

        return 1;
    }

    INSTRUCTION_EXECUTE(LD_DT_V) {
        ch8.DT.Set(ch8.V[op.X()].Get());

        return 1;
    }

    INSTRUCTION_EXECUTE(LD_ST_V) {
        ch8.ST.Set(ch8.V[op.X()].Get());

        return 1;
    }

    INSTRUCTION_EXECUTE(ADD_I_V) {
        ch8.I.Increment(ch8.V[op.X()].Get());

        return 1;
    }

    INSTRUCTION_EXECUTE(LD_F_V) {
        ch8.I.Set(Chip8::DigitSpace.start + sizeof(Digit) * ch8.V[op.X()].Get());

        return 1;
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

    INSTRUCTION_EXECUTE(LD_DEREF_I_V) {
        const auto &addr = ch8.I.Get();

        for (const auto offset : std::views::iota(0, op.X() + 1)) {
            const auto &reg = ch8.V[offset];

            ch8.memory[addr + offset] = static_cast<std::byte>(reg.Get());
        }

        return 1;
    }

    INSTRUCTION_EXECUTE(LD_V_DEREF_I) {
        const auto &addr = ch8.I.Get();

        for (const auto offset : std::views::iota(0, op.X() + 1)) {
            auto &reg = ch8.V[offset];

            reg.Set(static_cast<std::uint8_t>(ch8.memory[addr + offset]));
        }

        return 1;
    }

    #undef INSTRUCTION_EXECUTE

}
