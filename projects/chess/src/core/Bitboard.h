#pragma once
#include <cstdint>

namespace cave::chess {

struct Square {
    uint8_t val;

    bool IsValid() const { return val < 64; }
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

}  // namespace cave::chess
