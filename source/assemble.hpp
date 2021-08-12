#pragma once

#include "common.hpp"
#include "util.hpp"
#include "instruction.hpp"
#include "chip8.hpp"

namespace tsh {

    class Assembler {
        NON_COPYABLE(Assembler);
        NON_MOVEABLE(Assembler);

        public:
            static constexpr std::string_view CommentPrefix = "//";

            [[nodiscard]]
            static constexpr Address SizeForInstruction(const std::string_view ins) {
                return Chip8::Instructions::Size(ins);
            }

            template<std::integral T>
            [[nodiscard]]
            static constexpr std::optional<T> ToNumber(std::string_view str) {
                const auto base = [&]() {
                    if (str.starts_with("0x")) {
                        return std::size_t{16};
                    }

                    if (str.starts_with("0b")) {
                        return std::size_t{2};
                    }

                    if (str.starts_with("0o")) {
                        return std::size_t{8};
                    }

                    return std::size_t{10};
                }();

                if (base != 10) {
                    str = str.substr(2);
                }

                const char upper_char = std::min(static_cast<char>('0' + base), '9');

                T parsed = {};
                for (const auto &&[i, c] : util::enumerate(str)) {
                    const auto place = util::pow(base, str.size() - i - 1);

                    if (c >= '0' && c <= upper_char) {
                        parsed += place * ((c - '0') + 0);
                        continue;
                    }

                    if (base != 16) {
                        return {};
                    }

                    if (c >= 'a' && c <= 'f') {
                        parsed += place * ((c - 'a') + 0xa);
                        continue;
                    }

                    if (c >= 'A' && c <= 'F') {
                        parsed += place * ((c - 'A') + 0xA);
                        continue;
                    }

                    return {};
                }

                return parsed;
            }

            [[nodiscard]]
            static constexpr std::optional<std::uint8_t> RegisterNibble(const std::string_view str) {
                if (str.size() != 1) {
                    return {};
                }

                const char &nibble = str[0];

                if (nibble >= '0' && nibble <= '9') {
                    return nibble - '0' + 0x0;
                }

                if (nibble >= 'A' && nibble <= 'F') {
                    return nibble - 'A' + 0xA;
                }

                return {};
            }

            std::unordered_map<std::string_view, Address> labels;

            [[nodiscard]]
            std::optional<Address> ToAddress(const std::string_view address) const;

            ALWAYS_INLINE Assembler() = default;

            [[nodiscard]]
            std::optional<std::vector<std::byte>> Assemble(const std::string_view code);

            [[nodiscard]]
            std::optional<std::vector<std::byte>> AssembleFromFile(const std::string &path);
    };

    static_assert(Assembler::SizeForInstruction(".byte 0xCC") == 1);

    static_assert(Assembler::ToNumber<std::uint8_t>("0xCC") == 0xCC);

    static_assert(Assembler::RegisterNibble("F")  == 0xF);
    static_assert(Assembler::RegisterNibble("f")  == std::nullopt);
    static_assert(Assembler::RegisterNibble("FF") == std::nullopt);

}
