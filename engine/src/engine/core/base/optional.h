#pragma once
#include <optional>

namespace cave {

template<typename T>
struct SomeWrapper {
    T value;

    template<typename U>
    explicit SomeWrapper(U&& v)
        : value(std::forward<U>(v)) {}
};

template<typename T>
auto Some(T&& value) {
    return SomeWrapper<std::decay_t<T>>(std::forward<T>(value));
}

struct NoneWrapper {};

inline NoneWrapper None() {
    return {};
}

template<typename T>
class Option {
public:
    explicit Option()
        : m_opt(std::nullopt) {}

    template<typename U>
    Option(SomeWrapper<U>&& some)
        : m_opt(std::forward<U>(some.value)) {}

    Option(NoneWrapper)
        : m_opt(std::nullopt) {}

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

    T& unwrap_unchecked() {
        return *m_opt;
    }

    const T& unwrap_unchecked() const {
        return *m_opt;
    }

    T unwrap_or(T default_value) const {
        return m_opt.value_or(std::move(default_value));
    }

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
