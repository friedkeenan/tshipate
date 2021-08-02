#pragma once

#include "common.hpp"
#include "util.hpp"
#include "instruction.hpp"
#include "chip8.hpp"

namespace tsh {

    class Disassembler {
        NON_COPYABLE(Disassembler);
        NON_MOVEABLE(Disassembler);

        public:
            using Handler = Chip8::Handler;

            std::array<std::byte, Chip8::ProgramSpace.Size()> raw_program;

            ALWAYS_INLINE constexpr Disassembler() = default;

            template<typename... Args>
            [[nodiscard]]
            ALWAYS_INLINE constexpr std::optional<std::size_t> LoadProgram(Args &&... args) {
                return Chip8::LoadProgramIntoBuffer(this->raw_program.begin(), std::forward<Args>(args)...);
            }

            ALWAYS_INLINE static constexpr std::size_t ProgramOffset(const std::size_t offset) {
                return offset + Chip8::ProgramSpace.start;
            }

            ALWAYS_INLINE static DisassembleOutputIterator DisassembleByte(const DisassembleOutputIterator out, const std::size_t offset, const std::byte byte) {
                return fmt::format_to(out, "{0:04X}: ({1:02X})   -> .byte 0x{1:02X}\n", offset, byte);
            }

            template<typename... Args>
            [[nodiscard]]
            std::optional<std::string> Disassemble(Args &&... args) {
                const auto size_opt = this->LoadProgram(std::forward<Args>(args)...);
                if (!size_opt.has_value()) {
                    return {};
                }

                const auto program = std::span(this->raw_program.data(), *size_opt);

                std::string disassembly;
                auto inserter = std::back_inserter(disassembly);

                for (std::size_t i = 0; i < program.size(); i += sizeof(Opcode)) {
                    if (i + sizeof(Opcode) - 1 < program.size()) {
                        const auto opcode_data = program.subspan(i, sizeof(Opcode));
                        const auto op          = Opcode(Chip8::ReadRawOpcodeFromBuffer(opcode_data));

                        const auto new_it = Handler::Disassemble(inserter, ProgramOffset(i), op);
                        if (!new_it.has_value()) {
                            for (const auto &&[offset, byte] : util::enumerate(opcode_data)) {
                                inserter = DisassembleByte(inserter, ProgramOffset(offset + i), byte);
                            }
                        }
                    } else {
                        const auto data = program.subspan(i);
                        for (const auto &&[offset, byte] : util::enumerate(data)) {
                            inserter = DisassembleByte(inserter, ProgramOffset(offset + i), byte);
                        }
                    }
                }

                return disassembly;
            }
    };

}
