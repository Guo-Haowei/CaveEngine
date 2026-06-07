// =============================================================================
// File: cave/core/time/Nanoseconds.h
// =============================================================================
#pragma once
#include <cstdint>
#include <format>
#include <string>

namespace cave {

constexpr inline uint64_t kNsPerMs = 1'000'000ull;
constexpr inline uint64_t kNsPerSec = 1'000'000'000ull;

struct Nanoseconds {
    uint64_t value;

    constexpr Nanoseconds() noexcept
        : value(0) {}

    constexpr explicit Nanoseconds(uint64_t p_ns) noexcept
        : value(p_ns) {}

    double ToMilliseconds() const {
        return (double)value / kNsPerMs;
    }

    double ToSeconds() const {
        return (double)value / kNsPerSec;
    }

    std::string ToString() const {
        if (value < (kNsPerSec / 10)) {
            return std::format("{:.2f}ms", ToMilliseconds());
        }

        return std::format("{:.2f}seconds", ToSeconds());
    }

    Nanoseconds operator+(const Nanoseconds& p_other) const {
        return Nanoseconds(value + p_other.value);
    }

    Nanoseconds operator-(const Nanoseconds& p_other) const {
        return Nanoseconds(value - p_other.value);
    }

    Nanoseconds& operator+=(const Nanoseconds& p_other) {
        value += p_other.value;
        return *this;
    }

    Nanoseconds& operator-=(const Nanoseconds& p_other) {
        value -= p_other.value;
        return *this;
    }
};

}  // namespace cave
