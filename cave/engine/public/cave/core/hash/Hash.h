// =============================================================================
// File: engine/public/cave/core/hash/Hash.h
// =============================================================================
#pragma once
#include <type_traits>

namespace cave {

struct Hash {
    static constexpr inline void Combine(size_t& p_inout, size_t p_v) noexcept {
        p_inout ^= p_v + 0x9e3779b97f4a7c15ull + (p_inout << 6) + (p_inout >> 2);
    }

    // Hash a value using std::hash, with special-case for enums.
    template<typename T>
    static inline size_t Of(const T& p_v) noexcept {
        if constexpr (std::is_enum_v<T>) {
            using U = std::underlying_type_t<T>;
            return std::hash<U>{}(static_cast<U>(p_v));
        } else {
            return std::hash<T>{}(p_v);
        }
    }

    template<typename T>
    static inline void Add(size_t& p_inout, const T& p_v) noexcept {
        Combine(p_inout, Of(p_v));
    }

    // Hash multiple fields in order.
    template<typename... Ts>
    static inline size_t Many(const Ts&... p_vs) noexcept {
        size_t h = 0;
        (Add(h, p_vs), ...);
        return h;
    }

    static constexpr inline uint64_t Hash64(std::string_view s) {
        uint64_t h = 14695981039346656037ull;
        for (unsigned char c : s) {
            h ^= (uint64_t)c;
            h *= 1099511628211ull;
        }
        return h;
    }
};

}  // namespace cave
