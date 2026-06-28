#include "cave/core/math/AABB.h"

namespace cave::math {

TEST(aabb, from_center_size) {
    AABB aabb = AABB::fromCenterSize(Vec3f(10, 8, 6), Vec3f(4));
    EXPECT_EQ(aabb.center(), Vec3f(10, 8, 6));
    EXPECT_EQ(aabb.size(), Vec3f(4));
    EXPECT_EQ(aabb.min(), Vec3f(8, 6, 4));
    EXPECT_EQ(aabb.max(), Vec3f(12, 10, 8));
}

}  // namespace cave::math
