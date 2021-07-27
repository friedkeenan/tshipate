#pragma once

#define NON_COPYABLE(cls) \
    cls(const cls &) = delete; \
    cls &operator =(const cls &) = delete

#define NON_MOVEABLE(cls) \
    cls(cls &&) = delete; \
    cls &operator =(cls &&) = delete

#define ALWAYS_INLINE [[gnu::always_inline]] inline

#define CONCATENATE_IMPL(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_IMPL(s1, s2)

#if defined(__COUNTER__)

#define ANONYMOUS_VARIABLE(prefix) CONCATENATE(prefix, __COUNTER__)

#else

#define ANONYMOUS_VARIABLE(prefix) CONCATENATE(prefix, __LINE__)

#endif

#define BITSIZEOF(x) sizeof(x) * CHAR_BIT

namespace tsh::impl {

    template<typename... Args>
    ALWAYS_INLINE constexpr void Unused(Args &&... args) {
        (static_cast<void>(args), ...);
    }

    void Error(const char *msg);

}

#define UNUSED(...) ::tsh::impl::Unused(__VA_ARGS__)

#define ERROR(x) ::tsh::impl::Error(x)
