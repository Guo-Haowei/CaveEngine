#include "Square.h"

namespace chess::core {

std::tuple<uint8_t, uint8_t> Square::FileRank() const {
    const uint8_t file = m_index & 7;
    const uint8_t rank = m_index >> 3;
    return std::make_tuple(file, rank);
}

// Shoelace Formula (also called the Surveyor's Formula) for the area of a triangle in 2D space.
// area = [ Ax * (By - Cy) + Bx * (Cy - Ay) + Cx * (Ay - By) ] / 2
// but we only cares about the sign of the area, so we can skip the division by 2.
bool Square::SameLineInclusive(Square a, Square b) const {
    const auto [ax, ay] = a.FileRank();
    const auto [bx, by] = b.FileRank();
    const auto [cx, cy] = FileRank();

    const int two_signed_area =
        (int)ax * ((int)by - (int)cy) +
        (int)bx * ((int)cy - (int)ay) +
        (int)cx * ((int)ay - (int)by);

    const bool same_line = two_signed_area == 0;
    if (!same_line) return false;

    uint8_t x_min = ax, x_max = bx;
    if (x_min > x_max) std::swap(x_min, x_max);
    uint8_t y_min = ay, y_max = by;
    if (y_min > y_max) std::swap(y_min, y_max);

    const bool between_x = cx >= x_min && cx <= x_max;
    const bool between_y = cy >= y_min && cy <= y_max;
    return between_x && between_y;
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

    return kSquareLookUp[m_index];
}

#if defined(CAVE_TEST)

[[maybe_unused]] static constexpr Square A1(0);
[[maybe_unused]] static constexpr Square B1(1);
[[maybe_unused]] static constexpr Square C1(2);
[[maybe_unused]] static constexpr Square D1(3);
[[maybe_unused]] static constexpr Square E1(4);
[[maybe_unused]] static constexpr Square F1(5);
[[maybe_unused]] static constexpr Square G1(6);
[[maybe_unused]] static constexpr Square H1(7);

[[maybe_unused]] static constexpr Square A2(8);
[[maybe_unused]] static constexpr Square B2(9);
[[maybe_unused]] static constexpr Square C2(10);
[[maybe_unused]] static constexpr Square D2(11);
[[maybe_unused]] static constexpr Square E2(12);
[[maybe_unused]] static constexpr Square F2(13);
[[maybe_unused]] static constexpr Square G2(14);
[[maybe_unused]] static constexpr Square H2(15);

[[maybe_unused]] static constexpr Square A3(16);
[[maybe_unused]] static constexpr Square B3(17);
[[maybe_unused]] static constexpr Square C3(18);
[[maybe_unused]] static constexpr Square D3(19);
[[maybe_unused]] static constexpr Square E3(20);
[[maybe_unused]] static constexpr Square F3(21);
[[maybe_unused]] static constexpr Square G3(22);
[[maybe_unused]] static constexpr Square H3(23);

[[maybe_unused]] static constexpr Square A4(24);
[[maybe_unused]] static constexpr Square B4(25);
[[maybe_unused]] static constexpr Square C4(26);
[[maybe_unused]] static constexpr Square D4(27);
[[maybe_unused]] static constexpr Square E4(28);
[[maybe_unused]] static constexpr Square F4(29);
[[maybe_unused]] static constexpr Square G4(30);
[[maybe_unused]] static constexpr Square H4(31);

[[maybe_unused]] static constexpr Square A5(32);
[[maybe_unused]] static constexpr Square B5(33);
[[maybe_unused]] static constexpr Square C5(34);
[[maybe_unused]] static constexpr Square D5(35);
[[maybe_unused]] static constexpr Square E5(36);
[[maybe_unused]] static constexpr Square F5(37);
[[maybe_unused]] static constexpr Square G5(38);
[[maybe_unused]] static constexpr Square H5(39);

[[maybe_unused]] static constexpr Square A6(40);
[[maybe_unused]] static constexpr Square B6(41);
[[maybe_unused]] static constexpr Square C6(42);
[[maybe_unused]] static constexpr Square D6(43);
[[maybe_unused]] static constexpr Square E6(44);
[[maybe_unused]] static constexpr Square F6(45);
[[maybe_unused]] static constexpr Square G6(46);
[[maybe_unused]] static constexpr Square H6(47);

[[maybe_unused]] static constexpr Square A7(48);
[[maybe_unused]] static constexpr Square B7(49);
[[maybe_unused]] static constexpr Square C7(50);
[[maybe_unused]] static constexpr Square D7(51);
[[maybe_unused]] static constexpr Square E7(52);
[[maybe_unused]] static constexpr Square F7(53);
[[maybe_unused]] static constexpr Square G7(54);
[[maybe_unused]] static constexpr Square H7(55);

[[maybe_unused]] static constexpr Square A8(56);
[[maybe_unused]] static constexpr Square B8(57);
[[maybe_unused]] static constexpr Square C8(58);
[[maybe_unused]] static constexpr Square D8(59);
[[maybe_unused]] static constexpr Square E8(60);
[[maybe_unused]] static constexpr Square F8(61);
[[maybe_unused]] static constexpr Square G8(62);
[[maybe_unused]] static constexpr Square H8(63);

TEST(Square, SameLineDiagonal) {
    {
        const Square a = A1;
        const Square b = B2;
        const Square c = C3;

        EXPECT_FALSE(a.SameLineInclusive(b, c));
        EXPECT_TRUE(b.SameLineInclusive(a, c));
        EXPECT_FALSE(c.SameLineInclusive(a, b));
    }

    {
        const Square a = A1;
        const Square b = B1;
        const Square c = C3;

        EXPECT_FALSE(a.SameLineInclusive(b, c));
        EXPECT_FALSE(b.SameLineInclusive(a, c));
        EXPECT_FALSE(c.SameLineInclusive(a, b));
    }
}

TEST(Square, SameLineOverlapping) {
    const Square a = B2;
    const Square b = B2;
    const Square c = D8;

    EXPECT_FALSE(c.SameLineInclusive(a, b));
    EXPECT_TRUE(a.SameLineInclusive(b, c));
    EXPECT_TRUE(b.SameLineInclusive(a, c));
}

TEST(Square, SameLineHorizontal) {
    const Square a = C1;
    const Square b = C2;
    const Square c = C5;

    EXPECT_FALSE(a.SameLineInclusive(b, c));
    EXPECT_TRUE(b.SameLineInclusive(a, c));
    EXPECT_FALSE(c.SameLineInclusive(a, b));
}

TEST(Square, SameLineVertical) {
    const Square a = A1;
    const Square b = A8;
    const Square c = A3;

    EXPECT_FALSE(a.SameLineInclusive(b, c));
    EXPECT_FALSE(b.SameLineInclusive(a, c));
    EXPECT_TRUE(c.SameLineInclusive(a, b));
}

TEST(Square, MoreSameLineTest) {
    const Square a = G8;
    const Square b = B3;

    EXPECT_TRUE(F7.SameLineInclusive(a, b));
    EXPECT_TRUE(D5.SameLineInclusive(a, b));
    EXPECT_TRUE(C4.SameLineInclusive(a, b));
}
#endif

}  // namespace chess::core
