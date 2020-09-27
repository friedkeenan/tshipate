#pragma once

#define NON_COPYABLE(cls) \
    cls(const cls&) = delete; \
    cls& operator=(const cls&) = delete

#define NON_MOVEABLE(cls) \
    cls(cls&&) = delete; \
    cls& operator=(cls&&) = delete

#define NORETURN    __attribute__((noreturn))
#define WEAK_SYMBOL __attribute__((weak))
#define ALWAYS_INLINE_LAMBDA __attribute__((always_inline))
#define ALWAYS_INLINE inline __attribute__((always_inline))

#define CONCATENATE_IMPL(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_IMPL(s1, s2)

#ifdef __COUNTER__
#define ANONYMOUS_VARIABLE(pref) CONCATENATE(pref, __COUNTER__)
#else
#define ANONYMOUS_VARIABLE(pref) CONCATENATE(pref, __LINE__)
#endif

#define PREDICT(expr, value, _probability) __builtin_expect_with_probability(expr, value, ({ \
                                                    constexpr double probability = _probability; \
                                                    static_assert(0.0 <= probability);           \
                                                    static_assert(probability <= 1.0);           \
                                                    probability;                                 \
                                               }))

#define PREDICT_TRUE(expr, probability)  PREDICT(!!(expr), 1, probability)
#define PREDICT_FALSE(expr, probability) PREDICT(!!(expr), 0, probability)

#define LIKELY(expr)   PREDICT_TRUE(expr, 1.0)
#define UNLIKELY(expr) PREDICT_FALSE(expr, 1.0)
