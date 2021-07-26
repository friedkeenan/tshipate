#pragma once

#include "common.hpp"

namespace tsh {

    /* Thanks C++ Weekly */
    template<std::integral Internal = std::uint64_t>
    class BitPattern {
        public:
            static constexpr char IgnoreCharacter = '\'';

            Internal expected = {};
            Internal mask     = std::numeric_limits<Internal>::max();

            template<std::size_t N>
            consteval BitPattern(const char (&literal_str)[N]) {
                const auto pattern = std::string_view(literal_str, N - 1);

                auto bit = pattern.size() - std::ranges::count(pattern, IgnoreCharacter) - 1;
                for (const char &c : pattern) {
                    if (c == IgnoreCharacter) {
                        continue;
                    }

                    if (c == '1') {
                        this->expected |= Internal{1} << bit;
                        this->mask     |= Internal{1} << bit;
                    } else if (c == '0') {
                        this->expected |= Internal{0} << bit;
                        this->mask     |= Internal{1} << bit;
                    } else if (c == 'x') {
                        this->expected |=   Internal{0} << bit;
                        this->mask     &= ~(Internal{1} << bit);
                    } else {
                        ERROR("Invalid character for BitPattern");
                    }

                    bit--;
                }
            }

            template<std::integral T>
            ALWAYS_INLINE constexpr bool matches(const T value) {
                return (value & this->mask) == this->expected;
            }
    };

    static_assert(BitPattern("11x00").matches(0b11000));
    static_assert(BitPattern("11x00").matches(0b11100));
    static_assert(!BitPattern("11x00").matches(0b11010));

    static_assert(BitPattern("1111'xxxx'0000").matches(0b1111'1010'0000));

}
