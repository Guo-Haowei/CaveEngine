#include "Bitboard.h"

namespace chess::core {

#if defined(CAVE_TEST)

TEST(Bitboard, set_and_unset_same_square_does_nothing) {
    Bitboard bb{};
    EXPECT_TRUE(bb.Empty());

    bb.Set(Square(9));
    EXPECT_TRUE(bb.Any());

    bb.Unset(Square(9));
    EXPECT_TRUE(bb.Empty());
}

TEST(Bitboard, test_iterator) {
    constexpr Bitboard MASK_E{ 0x1010101010101010 };

    std::vector<Square> squares;
    for (Square sq : MASK_E.Squares()) {
        squares.push_back(sq);
    }

    ASSERT_EQ(squares.size(), 8);
    EXPECT_STREQ(squares[0].ToString(), "e1");
    EXPECT_STREQ(squares[1].ToString(), "e2");
    EXPECT_STREQ(squares[2].ToString(), "e3");
    EXPECT_STREQ(squares[3].ToString(), "e4");
    EXPECT_STREQ(squares[4].ToString(), "e5");
    EXPECT_STREQ(squares[5].ToString(), "e6");
    EXPECT_STREQ(squares[6].ToString(), "e7");
    EXPECT_STREQ(squares[7].ToString(), "e8");
}

#endif

}  // namespace chess::core
