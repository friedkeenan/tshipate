#include "common.hpp"

#include "chip8.hpp"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::printf("Invalid arguments, idiot.\n");

        return 1;
    }

    tsh::Chip8 ch8;
    if (!ch8.LoadProgram(argv[1])) {
        std::printf("Failed to load program!\n");
    }

    ch8.Loop();

    return 0;
}
