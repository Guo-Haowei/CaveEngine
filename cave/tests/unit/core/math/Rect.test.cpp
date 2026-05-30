#include "cave/core/math/Rect.h"

namespace cave::math {

// =============================================================================
// Rect::Top
// =============================================================================
TEST(Rect, left_top_bottom_right_calculation) {
    IntRect rect{ 1, 2, 3, 4 };
    EXPECT_EQ(rect.Left(), 1);
    EXPECT_EQ(rect.Right(), 4);

    EXPECT_EQ(rect.Top(), 2);
    EXPECT_EQ(rect.Bottom(), 6);
}

// =============================================================================
// Rect::FromMinMax
// =============================================================================
TEST(Rect, build_rect_from_min_max) {
    IntRect rect = IntRect::FromMinMax(1, 2, 3, 4);
    EXPECT_EQ(rect.Left(), 1);
    EXPECT_EQ(rect.Top(), 2);

    EXPECT_EQ(rect.Right(), 3);
    EXPECT_EQ(rect.Bottom(), 4);

    EXPECT_EQ(rect.w, 2);
    EXPECT_EQ(rect.h, 2);
}

// =============================================================================
// Rect::Contains
// =============================================================================
TEST(Rect, returns_true_if_point_is_inside) {
    IntRect rect{ 2, 4, 6, 8 };

    EXPECT_TRUE(rect.Contains(3, 5));
    EXPECT_TRUE(rect.Contains(7, 11));
}

TEST(Rect, returns_true_if_point_is_on_left_or_top_edge) {
    IntRect rect{ 2, 4, 6, 8 };

    EXPECT_TRUE(rect.Contains(2, 5));
    EXPECT_TRUE(rect.Contains(3, 4));
    EXPECT_TRUE(rect.Contains(2, 4));
}

TEST(Rect, returns_false_if_point_is_on_right_edge) {
    IntRect rect{ 2, 4, 6, 8 };

    // Right = x + w = 8
    EXPECT_FALSE(rect.Contains(8, 5));
}

TEST(Rect, returns_false_if_point_is_on_bottom_edge) {
    IntRect rect{ 2, 4, 6, 8 };

    // Bottom = y + h = 12
    EXPECT_FALSE(rect.Contains(3, 12));
}

TEST(Rect, returns_false_if_point_is_on_bottom_right_corner) {
    IntRect rect{ 2, 4, 6, 8 };

    EXPECT_FALSE(rect.Contains(8, 12));
}

TEST(Rect, returns_false_if_point_is_left_of_rect) {
    IntRect rect{ 2, 4, 6, 8 };

    EXPECT_FALSE(rect.Contains(1, 5));
}

TEST(Rect, returns_false_if_point_is_above_rect) {
    IntRect rect{ 2, 4, 6, 8 };

    EXPECT_FALSE(rect.Contains(3, 3));
}

TEST(Rect, returns_false_if_point_is_right_of_rect) {
    IntRect rect{ 2, 4, 6, 8 };

    EXPECT_FALSE(rect.Contains(9, 5));
}

TEST(Rect, returns_false_if_point_is_below_rect) {
    IntRect rect{ 2, 4, 6, 8 };

    EXPECT_FALSE(rect.Contains(3, 13));
}

TEST(Rect, adjacent_rects_do_not_both_contain_shared_edge_point) {
    IntRect left{ 0, 0, 10, 10 };
    IntRect right{ 10, 0, 10, 10 };

    EXPECT_FALSE(left.Contains(10, 5));
    EXPECT_TRUE(right.Contains(10, 5));
}

}  // namespace cave::math
