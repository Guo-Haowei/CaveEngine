// =============================================================================
// File: cave/core/hash/Hash.h
// =============================================================================
#pragma once
#include <string_view>
#include <type_traits>

namespace cave {

struct Hash {
    static constexpr void combine(size_t& inout, size_t v) noexcept {
        inout ^= v + 0x9e3779b97f4a7c15ull + (inout << 6) + (inout >> 2);
    }

    // Hash a value using std::hash, with special-case for enums.
    template<typename T>
    static size_t of(const T& v) noexcept {
        if constexpr (std::is_enum_v<T>) {
            using U = std::underlying_type_t<T>;
            return std::hash<U>{}(static_cast<U>(v));
        } else {
            return std::hash<T>{}(v);
        }
    }

    template<typename T>
    static void add(size_t& inout, const T& v) noexcept {
        combine(inout, of(v));
    }

    // Hash multiple fields in order.
    template<typename... Ts>
    static size_t many(const Ts&... args) noexcept {
        size_t h = 0;
        (add(h, args), ...);
        return h;
    }

    static constexpr inline uint64_t hash64(std::string_view s) {
        uint64_t h = 14695981039346656037ull;
        for (unsigned char c : s) {
            h ^= (uint64_t)c;
            h *= 1099511628211ull;
        }
        return h;
    }
};

}  // namespace cave
