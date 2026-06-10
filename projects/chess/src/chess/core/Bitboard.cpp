#include "Bitboard.h"

namespace chess::core {

#if defined(CAVE_TEST)

TEST(Bitboard, set_and_unset_same_square_does_nothing) {
    Bitboard bb{};
    EXPECT_TRUE(bb.Empty());

    bb.Set(Square::B2);
    EXPECT_TRUE(bb.Any());

    bb.Unset(Square::B2);
    EXPECT_TRUE(bb.Empty());
}

TEST(Bitboard, test_iterator) {
    constexpr Bitboard MASK_E{ 0x1010101010101010 };

    std::vector<Square> squares;
    for (Square sq : MASK_E.Squares()) {
        squares.push_back(sq);
    }

    ASSERT_EQ(squares.size(), 8);
    EXPECT_STREQ(squares[0].uci(), "e1");
    EXPECT_STREQ(squares[1].uci(), "e2");
    EXPECT_STREQ(squares[2].uci(), "e3");
    EXPECT_STREQ(squares[3].uci(), "e4");
    EXPECT_STREQ(squares[4].uci(), "e5");
    EXPECT_STREQ(squares[5].uci(), "e6");
    EXPECT_STREQ(squares[6].uci(), "e7");
    EXPECT_STREQ(squares[7].uci(), "e8");
}

#endif

}  // namespace chess::core
