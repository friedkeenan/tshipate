#include "common.hpp"
#include "util.hpp"

namespace tsh::util {

    std::optional<std::vector<std::string_view>> WildcardCapture(const std::string_view pattern, const std::string_view str) {
        static constexpr char WildcardChar = '*';

        static constexpr auto extend_capture = [](const std::string_view capture) {
            return std::string_view(capture.data(), capture.size() + 1);
        };

        /* If string is empty and the pattern is not, then the pattern doesn't match. */
        if (str.empty() && !pattern.empty()) {
            return {};
        }

        std::vector<std::string_view> captures;
        std::string_view current_capture;

        auto pattern_it = pattern.begin();
        for (const char &str_char : str) {
            /* If pattern is finished. */
            if (pattern_it == pattern.end()) {
                /* If there's not a capture going on, i.e. wildcard at the end. */
                if (current_capture.empty()) {
                    return {};
                }

                /*
                    If there is a wildcard at the end, extend the
                    current capture to the rest of the string.
                */
                current_capture = str.substr(current_capture.data() - str.data());
                break;
            }

            const char &pattern_char = *pattern_it;

            if (pattern_char == WildcardChar) {
                if (current_capture.empty()) {
                    /* Start a new capture. */
                    current_capture = std::string_view(&str_char, 1);
                } else {
                    /* Treat sequences of wildcards as one long capture. */
                    current_capture = extend_capture(current_capture);
                }

                pattern_it++;
                continue;
            }

            if (str_char == pattern_char) {
                /* If there is a capture occuring, push back and reset the current capture. */
                if (!current_capture.empty()) {
                    captures.push_back(current_capture);
                    current_capture = std::string_view();
                }

                pattern_it++;
                continue;
            }

            /* There's a capture going on, should capture characters that don't exactly match too. */
            if (!current_capture.empty()) {
                current_capture = extend_capture(current_capture);
                continue;
            }

            /* Pattern doesn't match. */
            return {};
        }

        /* Handle any captures at the end. */
        if (!current_capture.empty()) {
            captures.push_back(current_capture);
        }

        return captures;
    }

    bool WriteToFile(const std::string &path, const std::span<const std::byte> data) {
        const auto fp = std::fopen(path.c_str(), "wb");
        if (fp == nullptr) {
            return false;
        }

        ON_SCOPE_EXIT { std::fclose(fp); };

        if (std::fwrite(data.data(), data.size(), 1, fp) != 1) {
            return false;
        }

        return true;
    }

}
