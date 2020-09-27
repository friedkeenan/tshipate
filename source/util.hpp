#pragma once

/* From Atmosphere */
namespace tsh::util {

    template<class F>
    class ScopeGuard {
        NON_COPYABLE(ScopeGuard);
        private:
            F f;
            bool active;
        public:
            constexpr ALWAYS_INLINE ScopeGuard(F f) : f(std::move(f)), active(true) { }
            ALWAYS_INLINE ~ScopeGuard() { if (active) { f(); } }
            ALWAYS_INLINE void Cancel() { active = false; }

            ALWAYS_INLINE ScopeGuard(ScopeGuard&& rhs) : f(std::move(rhs.f)), active(rhs.active) {
                rhs.Cancel();
            }
    };

    template<class F>
    constexpr ALWAYS_INLINE ScopeGuard<F> MakeScopeGuard(F f) {
        return ScopeGuard<F>(std::move(f));
    }

    enum class ScopeGuardOnExit {};

    template <typename F>
    constexpr ALWAYS_INLINE ScopeGuard<F> operator+(ScopeGuardOnExit, F&& f) {
        return ScopeGuard<F>(std::forward<F>(f));
    }

}

#define SCOPE_GUARD   ::tsh::util::ScopeGuardOnExit() + [&]() ALWAYS_INLINE_LAMBDA
#define ON_SCOPE_EXIT auto ANONYMOUS_VARIABLE(SCOPE_EXIT_STATE_) = SCOPE_GUARD
