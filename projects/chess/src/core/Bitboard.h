#pragma once
#include <cstdint>
#include <tuple>

namespace chess::core {

class Square {
public:
    explicit Square(uint8_t p_val) noexcept
        : m_val(p_val) {
    }

    static Square From(uint8_t file, uint8_t rank) {
        const uint8_t val = rank * 8 + file;
        return Square(val);
    }

    bool IsValid() const { return m_val < 64; }

    uint8_t AsU8() const { return m_val; }

    std::tuple<uint8_t, uint8_t> FileRank() const;

    const char* ToString() const;

private:
    uint8_t m_val;
};

class Bitboard {
public:
    constexpr Bitboard()
        : m_val(0) {}

    explicit constexpr Bitboard(uint64_t p_val)
        : m_val(p_val) {}

    constexpr bool Empty() const { return m_val == 0; }
    constexpr bool Any() const { return m_val != 0; }

    bool Test(Square p_sq) const;
    void Set(Square p_sq);

    Bitboard operator|(const Bitboard& p_other) const {
        return Bitboard(m_val | p_other.m_val);
    }

private:
    uint64_t m_val;
};

}  // namespace chess::core
