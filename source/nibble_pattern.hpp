#pragma once

#include "common.hpp"

namespace tsh {

    template<std::integral Internal = std::uint64_t>
    class NibblePattern {
        public:
            static constexpr char IgnoreCharacter = '\'';

            Internal expected = {};
            Internal mask     = std::numeric_limits<Internal>::max();

            static consteval std::uint8_t CharToNibble(char c) {
                if (c >= '0' && c <= '9') {
                    return c - '0';
                }

                if (c >= 'A' && c <= 'F') {
                    return c - 'A' + 0xA;
                }

                ERROR("Invalid character for hex nibble");
                return 0xFF;
            }

            template<std::size_t N>
            explicit consteval NibblePattern(const char (&literal_str)[N]) {
                const auto pattern = std::string_view(literal_str, N - 1);

                auto nibble = pattern.size() - std::ranges::count(pattern, IgnoreCharacter) - 1;
                for (const char &c : pattern) {
                    if (c == IgnoreCharacter) {
                        continue;
                    }

                    if (c == 'x') {
                        this->expected |=  (Internal{0b0000} << (4 * nibble));
                        this->mask     &= ~(Internal{0b1111} << (4 * nibble));
                    } else {
                        const auto converted = static_cast<Internal>(CharToNibble(c));

                        this->expected |= (converted        << (4 * nibble));
                        this->mask     |= (Internal{0b1111} << (4 * nibble));
                    }

                    nibble--;
                }
            }

            template<std::integral T>
            ALWAYS_INLINE constexpr bool matches(T value) const {
                return (value & this->mask) == this->expected;
            }
    };

    static_assert(NibblePattern("CxFEBABE").matches(0xCAFEBABE));
    static_assert(NibblePattern("ABCD'xxxx'0123").matches(0xABCD'FFFF'0123));

}
