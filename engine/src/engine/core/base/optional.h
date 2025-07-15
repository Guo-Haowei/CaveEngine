#pragma once

namespace cave {

#include <optional>
#include <functional>
#include <stdexcept>

template<typename T>
class Option {
public:
    Option() = default;
    Option(std::nullopt_t)
        : m_opt(std::nullopt) {}
    Option(const T& value)
        : m_opt(value) {}
    Option(T&& value)
        : m_opt(std::move(value)) {}

    static Option Some(T value) { return Option(std::move(value)); }
    static Option None() { return Option(std::nullopt); }

    bool is_some() const { return m_opt.has_value(); }
    bool is_none() const { return !m_opt.has_value(); }

    T& unwrap() {
        if (!m_opt) CRASH_NOW_MSG("Called unwrap() on None");
        return *m_opt;
    }

    const T& unwrap() const {
        if (!m_opt) CRASH_NOW_MSG("Called unwrap() on None");
        return *m_opt;
    }

    T unwrap_or(T default_value) const {
        return m_opt.value_or(std::move(default_value));
    }

    template<typename F>
    auto map(F func) const -> Option<decltype(func(std::declval<T>()))> {
        if (m_opt)
            return Option<decltype(func(*m_opt))>::Some(func(*m_opt));
        else
            return Option<decltype(func(std::declval<T>()))>::None();
    }

    const std::optional<T>& as_std() const { return m_opt; }

    bool operator==(const Option& rhs) const {
        return m_opt == rhs.m_opt;
    }

    bool operator!=(const Option& rhs) const {
        return m_opt != rhs.m_opt;
    }

    bool operator==(const T& value) const {
        return m_opt == value;
    }

    bool operator!=(const T& value) const {
        return m_opt != value;
    }

private:
    std::optional<T> m_opt;
};

}  // namespace cave
