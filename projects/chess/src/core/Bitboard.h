#pragma once
#include <cstdint>
#include <tuple>

namespace chess::core {

struct Square {
    uint8_t val;

    explicit Square(uint8_t p_val) noexcept
        : val(p_val) {
    }

    bool IsValid() const { return val < 64; }

    static Square From(uint8_t file, uint8_t rank) {
        const uint8_t val = rank * 8 + file;
        return Square(val);
    }

    std::tuple<uint8_t, uint8_t> FileRank() const {
        const uint8_t file = val & 7;
        const uint8_t rank = val >> 3;
        return std::make_tuple(file, rank);
    }
};

class Bitboard {
public:
    explicit constexpr Bitboard()
        : m_val(0) {}

    explicit constexpr Bitboard(uint64_t p_val)
        : m_val(p_val) {}

    constexpr bool Empty() const { return m_val == 0; }
    constexpr bool Any() const { return m_val != 0; }

    bool Test(Square p_sq) const;
    void Set(Square p_sq);

private:
    uint64_t m_val;
};

}  // namespace chess::core
