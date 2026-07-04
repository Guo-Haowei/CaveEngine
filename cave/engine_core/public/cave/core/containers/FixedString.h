// =============================================================================
// File: cave/core/containers/FixedString.h
// =============================================================================
#pragma once
#include <cstdint>
#include <cstring>
#include <string_view>
#include <algorithm>

#include "cave/core/CoreExport.h"

namespace cave {

template<size_t N>
class FixedString {
    static_assert(N >= 2, "FixedString capacity must be >= 2");

public:
    using value_type = char;
    using size_type = uint32_t;

public:
    constexpr FixedString() noexcept {
        data_[0] = '\0';
        size_ = 0;
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
    const char* c_str() const noexcept { return data_; }
    const char* data() const noexcept { return data_; }
    char* data() noexcept { return data_; }

    size_type size() const noexcept { return size_; }
    size_type length() const noexcept { return size_; }
    constexpr size_type capacity() const noexcept { return (size_type)(N - 1); }

    bool empty() const noexcept { return size_ == 0; }

    void clear() noexcept {
        size_ = 0;
        data_[0] = '\0';
    }

    // -------------------------------------------------------------------------
    // String view conversion
    // -------------------------------------------------------------------------
    std::string_view view() const noexcept {
        return std::string_view(data_, size_);
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
            data_[i] = sv[i];
        }
        data_[n] = '\0';
        size_ = n;
    }

    // -------------------------------------------------------------------------
    // Append
    // -------------------------------------------------------------------------
    void append(std::string_view sv) noexcept {
        const size_type free_space = capacity() - size_;
        const size_type n = (size_type)std::min<size_t>(sv.size(), free_space);

        if (n > 0) {
            std::memcpy(data_ + size_, sv.data(), n);
            size_ += n;
            data_[size_] = '\0';
        }
    }

    FixedString& operator+=(std::string_view sv) noexcept {
        append(sv);
        return *this;
    }

    void push_back(char c) noexcept {
        if (size_ < capacity()) {
            data_[size_] = c;
            ++size_;
            data_[size_] = '\0';
        }
    }

    // -------------------------------------------------------------------------
    // Indexing
    // -------------------------------------------------------------------------
    char& operator[](size_type i) noexcept { return data_[i]; }
    const char& operator[](size_type i) const noexcept { return data_[i]; }

    char front() const noexcept { return data_[0]; }
    char back() const noexcept { return size_ ? data_[size_ - 1] : '\0'; }

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
        return a.size_ == b.size_ && std::memcmp(a.data_, b.data_, a.size_) == 0;
    }

    friend bool operator!=(const FixedString& a, std::string_view b) noexcept {
        return !(a == b);
    }

    friend bool operator!=(const FixedString& a, const FixedString& b) noexcept {
        return !(a == b);
    }

private:
    char data_[N]{};
    size_type size_ = 0;
};

}  // namespace cave