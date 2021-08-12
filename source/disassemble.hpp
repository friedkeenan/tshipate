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
            std::array<std::byte, Chip8::ProgramSpace.Size()> raw_program;

            ALWAYS_INLINE static constexpr Address ProgramAddress(const Address offset) {
                return Chip8::ProgramSpace.OffsetToAbsolute(offset);
            }

            ALWAYS_INLINE static DisassembleOutputIterator DisassembleByte(const DisassembleOutputIterator out, const Address address, const std::byte byte) {
                return fmt::format_to(out, "{0:04X}: ({1:02X})   -> .byte 0x{1:02X}\n", address, byte);
            }

            ALWAYS_INLINE constexpr Disassembler() = default;

            template<typename... Args>
            [[nodiscard]]
            ALWAYS_INLINE constexpr std::optional<std::size_t> LoadProgram(Args &&... args) {
                return Chip8::LoadProgramIntoBuffer(this->raw_program.begin(), std::forward<Args>(args)...);
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

                for (Address i = 0; i < program.size(); i += sizeof(Opcode)) {
                    const auto byte_data = std::span(program.data() + i, std::min(sizeof(Opcode), program.size() - i));
                    if (byte_data.size() == sizeof(Opcode)) {
                        const auto op = Opcode(Chip8::ReadRawOpcodeFromBuffer(byte_data));

                        /* If new_it has no value, move onto disassembling bytes. */
                        const auto new_it = Chip8::Instructions::Disassemble(inserter, ProgramAddress(i), op);
                        if (new_it.has_value()) {
                            inserter = *new_it;
                            continue;
                        }
                    }

                    for (const auto &&[offset, byte] : util::enumerate(byte_data)) {
                        inserter = DisassembleByte(inserter, ProgramAddress(offset + i), byte);
                    }
                }

                return disassembly;
            }
    };

}
