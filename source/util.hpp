#pragma once

namespace tsh::util {

    constexpr inline auto enumerate = std::views::transform([i = std::size_t{0}](auto &&x) mutable {
        const auto index = i;

        i++;

        return std::pair<std::size_t, decltype(x)>(index, x);
    });

    template<typename Key, typename Value, std::size_t N>
    class Map {
        public:
            using KeyAndValue = std::pair<Key, Value>;

            Value default_value;
            std::array<KeyAndValue, N> internal;

            template<std::same_as<KeyAndValue>... Entries> requires (sizeof...(Entries) == N)
            explicit constexpr Map(const Value &default_value, Entries &&... entries)
                : default_value(default_value), internal({std::forward<Entries>(entries)...}) { }

            ALWAYS_INLINE constexpr auto begin() {
                return this->internal.begin();
            }

            ALWAYS_INLINE constexpr auto begin() const {
                return this->internal.begin();
            }

            ALWAYS_INLINE constexpr auto end() {
                return this->internal.end();
            }

            ALWAYS_INLINE constexpr auto end() const {
                return this->internal.end();
            }

            ALWAYS_INLINE constexpr auto data() {
                return this->internal.data();
            }

            ALWAYS_INLINE constexpr auto data() const {
                return this->internal.data();
            }

            ALWAYS_INLINE constexpr auto size() const {
                return this->internal.size();
            }

            constexpr std::optional<Key> KeyForValue(const Value &to_find) const {
                for (const auto &[key, value] : *this) {
                    if (value == to_find) {
                        return key;
                    }
                }

                return {};
            }

            constexpr Value &operator [](const Key &to_find) {
                for (auto &[key, value] : *this) {
                    if (key == to_find) {
                        return value;
                    }
                }

                return this->default_value;
            }

            constexpr const Value &operator [](const Key &to_find) const {
                for (const auto &[key, value] : *this) {
                    if (key == to_find) {
                        return value;
                    }
                }

                return this->default_value;
            }
    };

    template<typename T>
    concept is_pair = requires {
        typename T::first_type;
        typename T::second_type;
    } && std::same_as<T, std::pair<typename T::first_type, typename T::second_type>>;

    template<typename Value, is_pair FirstEntry, std::same_as<FirstEntry>... Entries>
    requires std::same_as<Value, typename FirstEntry::second_type>
    Map(Value, FirstEntry, Entries...) -> Map<typename FirstEntry::first_type, Value, sizeof...(Entries) + 1>;

    /* From Atmosphere */
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
