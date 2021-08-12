#include "common.hpp"
#include "assemble.hpp"
#include "chip8.hpp"

namespace tsh {

    namespace {

        std::string TrimWhitespace(const std::string_view str) {
            /* Without our own function we get unresolved overloads. */
            static constexpr auto resolved_isspace = [](char c) {
                return static_cast<bool>(std::isspace(c));
            };

            auto view = str |
                std::views::drop_while(resolved_isspace) |
                std::views::reverse                      |
                std::views::drop_while(resolved_isspace) |
                std::views::reverse;

            return std::string(std::ranges::begin(view), std::ranges::end(view));
        }

        std::vector<std::string> TrimmedLines(const std::string_view code) {
            std::vector<std::string> lines;

            for (const auto line_rng : code | std::views::split('\n')) {
                /*
                    GCC 11.1 and 11.2 choke up on string_view and string constructors for iterators.
                    Even std::views::common doesn't fix it.

                    TODO: Make this not garbage.
                */
                const auto line = [&]() {
                    std::string line;
                    std::ranges::copy(line_rng, std::back_inserter(line));

                    const auto comment_pos = line.find(Assembler::CommentPrefix);

                    if (comment_pos != std::string::npos) {
                        line.erase(comment_pos);
                    }

                    return TrimWhitespace(line);
                }();

                if (!line.empty()) {
                    lines.push_back(line);
                }

            }

            return lines;
        }

        ALWAYS_INLINE constexpr bool IsLabel(const std::string_view line) {
            return line.ends_with(':');
        }

        std::unordered_map<std::string_view, Address> ParseLabels(const std::vector<std::string> &lines) {
            std::unordered_map<std::string_view, Address> labels;
            Address current_addr = Chip8::ProgramSpace.start;

            for (const auto &line : lines) {
                if (IsLabel(line)) {
                    auto label = std::string_view(line);
                    label.remove_suffix(1);

                    labels[label] = current_addr;

                    continue;
                }

                current_addr += Assembler::SizeForInstruction(line);
            }

            return labels;
        }

    }

    std::optional<Address> Assembler::ToAddress(const std::string_view address) const {
        if (address.empty()) {
            return {};
        }

        /* Try to find a label first. If that fails, interpret literal address. */
        const auto it = this->labels.find(address);
        if (it != this->labels.end()) {
            return it->second;
        }

        return ToNumber<Address>(address);
    }

    std::optional<std::vector<std::byte>> Assembler::Assemble(const std::string_view code) {
        const auto lines = TrimmedLines(code);
        this->labels     = ParseLabels(lines);

        std::vector<std::byte> program;

        for (const auto &line : lines) {
            if (IsLabel(line)) {
                continue;
            }

            const auto data = Chip8::Instructions::Assemble(*this, line);
            if (!data.has_value()) {
                fmt::print("Failed to assemble instruction: {}\n", line);
                return {};
            }

            program.insert(program.end(), data->begin(), data->end());
        }

        return program;
    }

    std::optional<std::vector<std::byte>> Assembler::AssembleFromFile(const std::string &path) {
        const auto fp = std::fopen(path.c_str(), "r");
        if (fp == nullptr) {
            return {};
        }

        ON_SCOPE_EXIT { std::fclose(fp); };

        if (std::fseek(fp, 0, SEEK_END) != 0) {
            return {};
        }

        const auto offset = std::ftell(fp);
        if (offset < 0) {
            return {};
        }

        const auto size = static_cast<std::size_t>(offset);

        if (std::fseek(fp, 0, SEEK_SET) != 0) {
            return {};
        }

        auto code = std::string(size, '\0');
        if (std::fread(code.data(), size, 1, fp) != 1) {
            return {};
        }

        return this->Assemble(code);
    }

}
