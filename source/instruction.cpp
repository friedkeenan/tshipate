#include "common.hpp"
#include "instruction.hpp"
#include "chip8.hpp"

namespace tsh {

    #define INSTRUCTION_EXECUTE(name) \
        std::int32_t name::Execute(Chip8 &ch8, const Opcode op)

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

    INSTRUCTION_EXECUTE(SE_Byte) {
        if (ch8.V[op.X()].Get() == op.Byte()) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_EXECUTE(SNE_Byte) {
        if (ch8.V[op.X()].Get() != op.Byte()) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_EXECUTE(LD_Byte) {
        ch8.V[op.X()].Set(op.Byte());

        return 1;
    }

    INSTRUCTION_EXECUTE(ADD_Byte) {
        ch8.V[op.X()].Increment(op.Byte());

        return 1;
    }

    INSTRUCTION_EXECUTE(LD_Addr) {
        ch8.I.Set(op.Addr());

        return 1;
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
        const auto key = static_cast<Key>(op.X());

        if (ch8.keyboard.KeyPressed(key)) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_EXECUTE(SKNP) {
        const auto key = static_cast<Key>(op.X());

        if (!ch8.keyboard.KeyPressed(key)) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_EXECUTE(LD_V_DT) {
        ch8.V[op.X()].Set(ch8.DT.Get());

        return 1;
    }

    INSTRUCTION_EXECUTE(LD_DT_V) {
        ch8.DT.Set(ch8.V[op.X()].Get());

        return 1;
    }

    INSTRUCTION_EXECUTE(ADD_I_V) {
        ch8.I.Increment(ch8.V[op.X()].Get());

        return 1;
    }

    #undef INSTRUCTION_EXECUTE

}
