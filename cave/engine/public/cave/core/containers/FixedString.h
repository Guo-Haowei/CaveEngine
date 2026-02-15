// =============================================================================
// File: engine/public/cave/core/containers/FixedString.h
// =============================================================================
#pragma once
#include <cstdint>
#include <cstring>
#include <string_view>
#include <algorithm>

namespace cave {

template<size_t N>
class FixedString {
    static_assert(N >= 2, "FixedString capacity must be >= 2");

public:
    using value_type = char;
    using size_type = uint32_t;

public:
    constexpr FixedString() noexcept {
        m_data[0] = '\0';
        m_size = 0;
    }

    constexpr FixedString(std::string_view sv) noexcept {
        assign(sv);
    }

    FixedString(const char* s) noexcept {
        assign(s ? std::string_view(s) : std::string_view());
    }

    // -------------------------------------------------------------------------
    // Basic access
    // -------------------------------------------------------------------------
    const char* c_str() const noexcept { return m_data; }
    const char* data() const noexcept { return m_data; }
    char* data() noexcept { return m_data; }

    size_type size() const noexcept { return m_size; }
    size_type length() const noexcept { return m_size; }
    constexpr size_type capacity() const noexcept { return (size_type)(N - 1); }

    bool empty() const noexcept { return m_size == 0; }

    void clear() noexcept {
        m_size = 0;
        m_data[0] = '\0';
    }

    // -------------------------------------------------------------------------
    // String view conversion
    // -------------------------------------------------------------------------
    std::string_view view() const noexcept {
        return std::string_view(m_data, m_size);
    }

    operator std::string_view() const noexcept {
        return view();
    }

    // -------------------------------------------------------------------------
    // Assignment
    // -------------------------------------------------------------------------
    FixedString& operator=(std::string_view sv) noexcept {
        assign(sv);
        return *this;
    }

    FixedString& operator=(const char* s) noexcept {
        assign(s ? std::string_view(s) : std::string_view());
        return *this;
    }

    constexpr void assign(std::string_view sv) noexcept {
        const size_type n = (size_type)std::min<size_t>(sv.size(), capacity());
        for (size_type i = 0; i < n; ++i) {
            m_data[i] = sv[i];
        }
        m_data[n] = '\0';
        m_size = n;
    }

    // -------------------------------------------------------------------------
    // Append
    // -------------------------------------------------------------------------
    void append(std::string_view sv) noexcept {
        const size_type free_space = capacity() - m_size;
        const size_type n = (size_type)std::min<size_t>(sv.size(), free_space);

        if (n > 0) {
            std::memcpy(m_data + m_size, sv.data(), n);
            m_size += n;
            m_data[m_size] = '\0';
        }
    }

    FixedString& operator+=(std::string_view sv) noexcept {
        append(sv);
        return *this;
    }

    void push_back(char c) noexcept {
        if (m_size < capacity()) {
            m_data[m_size] = c;
            ++m_size;
            m_data[m_size] = '\0';
        }
    }

    // -------------------------------------------------------------------------
    // Indexing
    // -------------------------------------------------------------------------
    char& operator[](size_type i) noexcept { return m_data[i]; }
    const char& operator[](size_type i) const noexcept { return m_data[i]; }

    char front() const noexcept { return m_data[0]; }
    char back() const noexcept { return m_size ? m_data[m_size - 1] : '\0'; }

    // -------------------------------------------------------------------------
    // Compare
    // -------------------------------------------------------------------------
    int compare(std::string_view sv) const noexcept {
        return view().compare(sv);
    }

    bool starts_with(std::string_view sv) const noexcept {
        return view().starts_with(sv);
    }

    bool ends_with(std::string_view sv) const noexcept {
        return view().ends_with(sv);
    }

    bool contains(std::string_view sv) const noexcept {
        return view().find(sv) != std::string_view::npos;
    }

    // -------------------------------------------------------------------------
    // Equality
    // -------------------------------------------------------------------------
    friend bool operator==(const FixedString& a, std::string_view b) noexcept {
        return a.view() == b;
    }

    friend bool operator==(std::string_view a, const FixedString& b) noexcept {
        return a == b.view();
    }

    friend bool operator==(const FixedString& a, const FixedString& b) noexcept {
        return a.m_size == b.m_size && std::memcmp(a.m_data, b.m_data, a.m_size) == 0;
    }

    friend bool operator!=(const FixedString& a, std::string_view b) noexcept {
        return !(a == b);
    }

    friend bool operator!=(const FixedString& a, const FixedString& b) noexcept {
        return !(a == b);
    }

private:
    char m_data[N]{};
    size_type m_size = 0;
};

}  // namespace cave