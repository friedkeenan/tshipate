#include <argparse/argparse.hpp>

#include "common.hpp"
#include "chip8.hpp"
#include "disassemble.hpp"

int main(int argc, char **argv) {
    argparse::ArgumentParser program("tshipate");

    program.add_argument("-d", "--disassemble")
        .help("Disassmeble the program")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("program_path")
        .help("The program to act on");

    program.parse_args(argc, argv);
    const auto program_path = program.get<std::string>("program_path");

    if (program.get<bool>("--disassemble")) {
        tsh::Disassembler dis;
        const auto str = dis.Disassemble(program_path);
        if (!str.has_value()) {
            std::printf("Failed to disassemble program!\n");
        }

        std::printf(str->c_str());
    } else {
        tsh::Chip8 ch8;
        if (!ch8.LoadProgram(program_path)) {
            std::printf("Failed to load program!\n");
        }

        ch8.Loop();
    }

    return 0;
}
