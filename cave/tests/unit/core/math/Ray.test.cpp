#include "cave/core/math/Plane.h"
#include "cave/core/math/Ray.h"

namespace cave::math {

TEST(Ray, intersets_with_unparallel_plane) {
    Ray ray(Vec3f::UnitY, -Vec3f::UnitY);
    EXPECT_TRUE(ray.intersects(Plane::xz()));
}

TEST(Ray, cannot_interset_with_parallel_plane) {
    Ray ray(Vec3f{ 0, 1, 0 }, Vec3f{ 1, 1, 0 });
    EXPECT_FALSE(ray.intersects(Plane::xz()));
}

TEST(Ray, cannot_interset_with_plane_behind) {
    Ray ray(Vec3f{ 0, 1, 0 }, Vec3f{ 0, 2, 0 });
    EXPECT_FALSE(ray.intersects(Plane::xz()));
}

TEST(Ray, intersets_with_triangle) {
    Ray ray(Vec3f::UnitY, -Vec3f::UnitY);
    EXPECT_TRUE(ray.intersects(
        Vec3f{ -1, 0, -1 },
        Vec3f{ -1, 0, +1 },
        Vec3f{ +1, 0, +1 }));
}

}  // namespace cave::math
