#pragma once

#include "common.hpp"
#include "util.hpp"

namespace tsh {

    class Digit {
        NON_COPYABLE(Digit);
        NON_MOVEABLE(Digit);

        public:
            static constexpr std::size_t Width  = 4;
            static constexpr std::size_t Height = 5;

            static constexpr std::size_t Padding = 1;
            static constexpr auto CharRowLength  = Width + 2 * Padding;

            std::array<std::byte, Height> raw_data = {};

            consteval Digit(std::array<const char[CharRowLength + 1], Height> char_rows) {
                for (const auto &&[row_index, char_row] : util::enumerate(char_rows)) {
                    const auto row = std::string_view(char_row, CharRowLength);
                    auto &raw_row  = this->raw_data[row_index];

                    auto bit = BITSIZEOF(std::byte);
                    for (const char &c : row) {
                        if (c == '*') {
                            raw_row |= static_cast<std::byte>(std::uint8_t{1} << bit);
                        } else if (c == ' ') {
                            raw_row |= static_cast<std::byte>(std::uint8_t{0} << bit);
                        } else {
                            ERROR("Invalid character for Digit pattern");
                        }

                        bit--;
                    }
                }
            }

            ALWAYS_INLINE constexpr auto begin() const {
                return this->raw_data.begin();
            }


            ALWAYS_INLINE constexpr auto end() const {
                return this->raw_data.end();
            }

            ALWAYS_INLINE constexpr auto data() const {
                return this->raw_data.data();
            }

            ALWAYS_INLINE constexpr auto size() const {
                return this->raw_data.size();
            }
    };

    static_assert(sizeof(Digit) == Digit::Height);

    constexpr inline std::array<Digit, 0x10> Digits = {
        Digit({
            " **** ",
            " *  * ",
            " *  * ",
            " *  * ",
            " **** ",
        }),

        Digit({
            "   *  ",
            "  **  ",
            "   *  ",
            "   *  ",
            "  *** ",
        }),

        Digit({
            " **** ",
            "    * ",
            " **** ",
            " *    ",
            " **** ",
        }),

        Digit({
            " **** ",
            "    * ",
            " **** ",
            "    * ",
            " **** ",
        }),

        Digit({
            " *  * ",
            " *  * ",
            " **** ",
            "    * ",
            "    * ",
        }),

        Digit({
            " **** ",
            " *    ",
            " **** ",
            "    * ",
            " **** ",
        }),

        Digit({
            " **** ",
            " *    ",
            " **** ",
            " *  * ",
            " **** ",
        }),

        Digit({
            " **** ",
            "    * ",
            "   *  ",
            "  *   ",
            "  *   ",
        }),

        Digit({
            " **** ",
            " *  * ",
            " **** ",
            " *  * ",
            " **** ",
        }),

        Digit({
            " **** ",
            " *  * ",
            " **** ",
            "    * ",
            " **** ",
        }),

        Digit({
            " **** ",
            " *  * ",
            " **** ",
            " *  * ",
            " *  * ",
        }),

        Digit({
            " ***  ",
            " *  * ",
            " ***  ",
            " *  * ",
            " ***  ",
        }),

        Digit({
            " **** ",
            " *    ",
            " *    ",
            " *    ",
            " **** ",
        }),

        Digit({
            " ***  ",
            " *  * ",
            " *  * ",
            " *  * ",
            " ***  ",
        }),

        Digit({
            " **** ",
            " *    ",
            " **** ",
            " *    ",
            " **** ",
        }),

        Digit({
            " **** ",
            " *    ",
            " **** ",
            " *    ",
            " *    ",
        }),
    };

    static_assert(Digits[0xF].raw_data == std::array{
        std::byte{0b11110000},
        std::byte{0b10000000},
        std::byte{0b11110000},
        std::byte{0b10000000},
        std::byte{0b10000000},
    });

}
