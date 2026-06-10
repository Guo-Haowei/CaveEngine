#include "Square.h"

namespace chess::core {

// Shoelace Formula (also called the Surveyor's Formula) for the area of a triangle in 2D space.
// area = [ Ax * (By - Cy) + Bx * (Cy - Ay) + Cx * (Ay - By) ] / 2
// but we only cares about the sign of the area, so we can skip the division by 2.
bool Square::sameLineInclusive(Square a, Square b) const {
    const auto [ax, ay] = a.fileRank();
    const auto [bx, by] = b.fileRank();
    const auto [cx, cy] = fileRank();

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

const char* Square::uci() const {
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

    return kSquareLookUp[index_];
}

constexpr Square Square::A1(0);
constexpr Square Square::B1(1);
constexpr Square Square::C1(2);
constexpr Square Square::D1(3);
constexpr Square Square::E1(4);
constexpr Square Square::F1(5);
constexpr Square Square::G1(6);
constexpr Square Square::H1(7);

constexpr Square Square::A2(8);
constexpr Square Square::B2(9);
constexpr Square Square::C2(10);
constexpr Square Square::D2(11);
constexpr Square Square::E2(12);
constexpr Square Square::F2(13);
constexpr Square Square::G2(14);
constexpr Square Square::H2(15);

constexpr Square Square::A3(16);
constexpr Square Square::B3(17);
constexpr Square Square::C3(18);
constexpr Square Square::D3(19);
constexpr Square Square::E3(20);
constexpr Square Square::F3(21);
constexpr Square Square::G3(22);
constexpr Square Square::H3(23);

constexpr Square Square::A4(24);
constexpr Square Square::B4(25);
constexpr Square Square::C4(26);
constexpr Square Square::D4(27);
constexpr Square Square::E4(28);
constexpr Square Square::F4(29);
constexpr Square Square::G4(30);
constexpr Square Square::H4(31);

constexpr Square Square::A5(32);
constexpr Square Square::B5(33);
constexpr Square Square::C5(34);
constexpr Square Square::D5(35);
constexpr Square Square::E5(36);
constexpr Square Square::F5(37);
constexpr Square Square::G5(38);
constexpr Square Square::H5(39);

constexpr Square Square::A6(40);
constexpr Square Square::B6(41);
constexpr Square Square::C6(42);
constexpr Square Square::D6(43);
constexpr Square Square::E6(44);
constexpr Square Square::F6(45);
constexpr Square Square::G6(46);
constexpr Square Square::H6(47);

constexpr Square Square::A7(48);
constexpr Square Square::B7(49);
constexpr Square Square::C7(50);
constexpr Square Square::D7(51);
constexpr Square Square::E7(52);
constexpr Square Square::F7(53);
constexpr Square Square::G7(54);
constexpr Square Square::H7(55);

constexpr Square Square::A8(56);
constexpr Square Square::B8(57);
constexpr Square Square::C8(58);
constexpr Square Square::D8(59);
constexpr Square Square::E8(60);
constexpr Square Square::F8(61);
constexpr Square Square::G8(62);
constexpr Square Square::H8(63);

#if defined(CAVE_TEST)

TEST(Square, same_line_diagonal) {
    {
        const Square a = Square::A1;
        const Square b = Square::B2;
        const Square c = Square::C3;

        EXPECT_FALSE(a.sameLineInclusive(b, c));
        EXPECT_TRUE(b.sameLineInclusive(a, c));
        EXPECT_FALSE(c.sameLineInclusive(a, b));
    }

    {
        const Square a = Square::A1;
        const Square b = Square::B1;
        const Square c = Square::C3;

        EXPECT_FALSE(a.sameLineInclusive(b, c));
        EXPECT_FALSE(b.sameLineInclusive(a, c));
        EXPECT_FALSE(c.sameLineInclusive(a, b));
    }
}

TEST(Square, same_line_overlapping) {
    const Square a = Square::B2;
    const Square b = Square::B2;
    const Square c = Square::D8;

    EXPECT_FALSE(c.sameLineInclusive(a, b));
    EXPECT_TRUE(a.sameLineInclusive(b, c));
    EXPECT_TRUE(b.sameLineInclusive(a, c));
}

TEST(Square, same_line_horizontal) {
    const Square a = Square::C1;
    const Square b = Square::C2;
    const Square c = Square::C5;

    EXPECT_FALSE(a.sameLineInclusive(b, c));
    EXPECT_TRUE(b.sameLineInclusive(a, c));
    EXPECT_FALSE(c.sameLineInclusive(a, b));
}

TEST(Square, same_line_vertical) {
    const Square a = Square::A1;
    const Square b = Square::A8;
    const Square c = Square::A3;

    EXPECT_FALSE(a.sameLineInclusive(b, c));
    EXPECT_FALSE(b.sameLineInclusive(a, c));
    EXPECT_TRUE(c.sameLineInclusive(a, b));
}

TEST(Square, more_same_line_test) {
    const Square a = Square::G8;
    const Square b = Square::B3;

    EXPECT_TRUE(Square::F7.sameLineInclusive(a, b));
    EXPECT_TRUE(Square::D5.sameLineInclusive(a, b));
    EXPECT_TRUE(Square::C4.sameLineInclusive(a, b));
}
#endif

}  // namespace chess::core
