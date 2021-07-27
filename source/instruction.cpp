#include "common.hpp"
#include "instruction.hpp"
#include "chip8.hpp"

namespace tsh {

    #define INSTRUCTION_EXECUTE(name) \
        std::int32_t name::Execute(Chip8 &ch8, Opcode op)

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

    #undef INSTRUCTION_EXECUTE

}
