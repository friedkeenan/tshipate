#include <argparse/argparse.hpp>

#include "common.hpp"
#include "util.hpp"
#include "chip8.hpp"
#include "disassemble.hpp"
#include "assemble.hpp"

int main(int argc, char **argv) {
    argparse::ArgumentParser program("tshipate");

    program.add_argument("-d", "--disassemble")
        .help("Disassmeble the rom")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("-a", "--assemble")
        .help("Assemble the argument");

    program.add_argument("rom_path")
        .help("The rom to act on");

    program.parse_args(argc, argv);
    const auto rom_path = program.get<std::string>("rom_path");

    if (program.present("--assemble")) {
        const auto to_assemble = program.get<std::string>("--assemble");

        tsh::Assembler assembler;
        const auto data = assembler.AssembleFromFile(to_assemble);
        if (!data.has_value()) {
            std::printf("Failed to assemble program!\n");
            return 1;
        }

        if (!tsh::util::WriteToFile(rom_path, std::span(*data))) {
            std::printf("Failed to write program to file!\n");
        }
    } else if (program.get<bool>("--disassemble")) {
        tsh::Disassembler dis;
        const auto str = dis.Disassemble(rom_path);
        if (!str.has_value()) {
            std::printf("Failed to disassemble program!\n");
            return 1;
        }

        std::printf(str->c_str());
    } else {
        tsh::Chip8 ch8;
        if (!ch8.LoadProgram(rom_path)) {
            std::printf("Failed to load program!\n");
            return 1;
        }

        ch8.Loop();
    }

    return 0;
}
