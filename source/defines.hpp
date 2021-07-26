#pragma once

#define ALWAYS_INLINE [[gnu::always_inline]] inline

namespace tsh::impl {

    template<typename... Args>
    ALWAYS_INLINE constexpr void Unused(Args &&... args) {
        (static_cast<void>(args), ...);
    }

    void Error(const char *msg);

}

#define UNUSED(...) ::tsh::impl::Unused(__VA_ARGS__)

#define ERROR(x) ::tsh::impl::Error(x)
