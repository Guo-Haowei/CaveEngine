#pragma once
#include <bit>
#include "Square.h"

namespace chess::core {

class BitboardSquares {
public:
    explicit constexpr BitboardSquares(uint64_t p_bb)
        : m_remaining(p_bb) {}

    struct Iterator {
        uint64_t remaining = 0;

        Square operator*() const {
            // remaining must be non-zero
            const uint8_t tz = static_cast<uint8_t>(std::countr_zero(remaining));
            return Square(tz);
        }

        Iterator& operator++() {
            // clear LSB
            remaining &= (remaining - 1);
            return *this;
        }

        constexpr bool operator!=(const Iterator& rhs) const {
            return remaining != rhs.remaining;
        }
    };

    constexpr Iterator begin() const { return Iterator{ m_remaining }; }
    constexpr Iterator end() const { return Iterator{ 0 }; }

private:
    uint64_t m_remaining = 0;
};

class Bitboard {
public:
    constexpr Bitboard()
        : m_bits(0) {}

    explicit constexpr Bitboard(uint64_t p_bits)
        : m_bits(p_bits) {}

    constexpr bool Empty() const { return m_bits == 0; }
    constexpr bool Any() const { return m_bits != 0; }

    bool Test(Square p_sq) const;
    void Set(Square p_sq);

    Bitboard operator|(const Bitboard& p_rhs) const {
        return Bitboard(m_bits | p_rhs.m_bits);
    }

    Bitboard operator&(const Bitboard& p_rhs) const {
        return Bitboard(m_bits & p_rhs.m_bits);
    }

    Bitboard operator~() const {
        return Bitboard(~m_bits);
    }

    constexpr BitboardSquares Squares() const {
        return BitboardSquares(m_bits);
    }

private:
    uint64_t m_bits;
};

}  // namespace chess::core
