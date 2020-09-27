#include <cstdio>

#include "chip8.hpp"
#include "util.hpp"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::printf("Invalid arguments, idiot\n");
        return 1;
    }

    tsh::Chip8 ch8;

    ch8.Initialize();
    ON_SCOPE_EXIT { ch8.Exit(); };

    if (!ch8.LoadProgram(argv[1])) {
        std::printf("Error loading program!\n");
        return 1;
    }

    ch8.Loop();

    return 0;
}
