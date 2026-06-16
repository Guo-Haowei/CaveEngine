#include "cave/core/math/AABB.h"

namespace cave::math {

TEST(aabb, from_center_size) {
    AABB aabb = AABB::FromCenterSize(Vec3f(10, 8, 6), Vec3f(4));
    EXPECT_EQ(aabb.Center(), Vec3f(10, 8, 6));
    EXPECT_EQ(aabb.Size(), Vec3f(4));
    EXPECT_EQ(aabb.Min(), Vec3f(8, 6, 4));
    EXPECT_EQ(aabb.Max(), Vec3f(12, 10, 8));
}

}  // namespace cave::math
