#include "cave/core/math/Rect.h"

namespace cave::math {

TEST(IntRect, left_top_bottom_right_calculation) {
    IntRect rect{ 1, 2, 3, 4 };
    EXPECT_EQ(rect.Left(), 1);
    EXPECT_EQ(rect.Right(), 4);

    EXPECT_EQ(rect.Top(), 2);
    EXPECT_EQ(rect.Bottom(), 6);
}

TEST(IntRect, from_min_max) {
    IntRect rect = IntRect::FromMinMax(1, 2, 3, 4);
    EXPECT_EQ(rect.Left(), 1);
    EXPECT_EQ(rect.Top(), 2);

    EXPECT_EQ(rect.Right(), 3);
    EXPECT_EQ(rect.Bottom(), 4);

    EXPECT_EQ(rect.w, 2);
    EXPECT_EQ(rect.h, 2);
}

}  // namespace cave::math
