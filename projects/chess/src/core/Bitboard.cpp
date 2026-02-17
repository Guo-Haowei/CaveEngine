#include "Bitboard.h"

#include "cave/core/ErrorMacros.h"

namespace chess::core {

std::tuple<uint8_t, uint8_t> Square::FileRank() const {
    const uint8_t file = m_val & 7;
    const uint8_t rank = m_val >> 3;
    return std::make_tuple(file, rank);
}

const char* Square::ToString() const {
    static constexpr const char kSquareLookUp[64][3] = {
        // clang-format off
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        // clang-format on
    };

    DEV_ASSERT_INDEX(m_val, 64);
    return kSquareLookUp[m_val];
}

bool Bitboard::Test(Square p_sq) const {
    DEV_ASSERT_MSG(p_sq.IsValid(), "square out of bounds");
    return m_val & (1llu << p_sq.AsU8());
}

void Bitboard::Set(Square p_sq) {
    DEV_ASSERT_MSG(p_sq.IsValid(), "square out of bounds");
    m_val |= (1llu << p_sq.AsU8());
}

#if defined(CAVE_TEST)

TEST(Bitboard, test_square) {
    Bitboard bb;
    bb.Set(Square(0));

    EXPECT_TRUE(bb.Test(Square(0)));
}

#endif

}  // namespace chess::core
