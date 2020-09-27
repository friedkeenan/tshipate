#include <cstdio>
#include <cstdint>
#include <cstring>

#include "instruction.hpp"
#include "chip8.hpp"

namespace tsh {

    #define INSTRUCTION_EXECUTE(name) \
        std::int64_t name::Execute(Chip8 &ch8, const Opcode<name::OpcodeType> &op)

    INSTRUCTION_EXECUTE(CLR) {
        std::memset(ch8.display, 0, sizeof(ch8.display));

        return 1;
    }

    INSTRUCTION_EXECUTE(RTS) {
        ch8.pc.Set(ch8.stack.top());
        ch8.stack.pop();

        return 1;
    }

    INSTRUCTION_EXECUTE(JP_Addr) {
        ch8.pc.Set(op.Addr());

        return 0;
    }

    INSTRUCTION_EXECUTE(CALL) {
        ch8.stack.push(ch8.pc.Get());
        ch8.pc.Set(op.Addr());

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
        auto &reg = ch8.V[op.X()];
        reg.Set(reg.Get() + op.Byte());

        return 1;
    }

    INSTRUCTION_EXECUTE(LD_V) {
        ch8.V[op.X()].Set(ch8.V[op.Y()].Get());

        return 1;
    }

    INSTRUCTION_EXECUTE(SNE_V) {
        if (ch8.V[op.X()].Get() != ch8.V[op.Y()].Get()) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_EXECUTE(LD_I) {
        ch8.I.Set(op.Addr());

        return 1;
    }

    INSTRUCTION_EXECUTE(RND) {
        ch8.V[op.X()].Set(ch8.RandomByte() & op.Byte());

        return 1;
    }

    INSTRUCTION_EXECUTE(DRW) {
        const auto collide = ch8.Draw(ch8.I.Get(), op.Nibble(), ch8.V[op.X()].Get(), ch8.V[op.Y()].Get());

        if (collide) {
            ch8.V[0xF].Set(1);
        } else {
            ch8.V[0xF].Set(0);
        }

        return 1;
    }

    INSTRUCTION_EXECUTE(SKP) {
        if (static_cast<std::uint8_t>(ch8.KeyPressed()) == ch8.V[op.X()].Get()) {
            return 2;
        }

        return 1;
    }

    INSTRUCTION_EXECUTE(SKNP) {
        if (static_cast<std::uint8_t>(ch8.KeyPressed()) != ch8.V[op.X()].Get()) {
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
        ch8.I.Set(ch8.I.Get() + ch8.V[op.X()].Get());

        return 1;
    }

}
