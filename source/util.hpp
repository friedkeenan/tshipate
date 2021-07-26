#pragma once

/* From Atmosphere */
namespace tsh::util {

    template<class F>
    class ScopeGuard {
        NON_COPYABLE(ScopeGuard);

        public:
            F f;
            bool active;

            ALWAYS_INLINE constexpr ScopeGuard(F f) : f(std::move(f)), active(true) { }
            ALWAYS_INLINE constexpr ~ScopeGuard() { if (active) { f(); } }
            ALWAYS_INLINE constexpr void Cancel() { active = false; }

            ALWAYS_INLINE constexpr ScopeGuard(ScopeGuard&& rhs) : f(std::move(rhs.f)), active(rhs.active) {
                rhs.Cancel();
            }
    };

    template<class F>
    ALWAYS_INLINE constexpr ScopeGuard<F> MakeScopeGuard(F f) {
        return ScopeGuard<F>(std::move(f));
    }

    enum class ScopeGuardOnExit {};

    template <typename F>
    ALWAYS_INLINE constexpr ScopeGuard<F> operator+(ScopeGuardOnExit, F&& f) {
        return ScopeGuard<F>(std::forward<F>(f));
    }

}

#define SCOPE_GUARD   ::tsh::util::ScopeGuardOnExit() + [&]()
#define ON_SCOPE_EXIT auto ANONYMOUS_VARIABLE(SCOPE_EXIT_STATE_) = SCOPE_GUARD
