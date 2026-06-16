#include "cave/core/math/Box.h"

namespace cave::math {

TEST(box, constructor) {
    Box3 box(Vec3f(1), Vec3f(10));
    EXPECT_EQ(box.Min(), Vec3f(1));
    EXPECT_EQ(box.Max(), Vec3f(10));
}

TEST(box, expand_point) {
    Box3 box;
    box.ExpandPoint(Vec3f(1));

    EXPECT_EQ(box.Min(), Vec3f(1));
    EXPECT_EQ(box.Max(), Vec3f(1));

    box.ExpandPoint(Vec3f(3));
    EXPECT_EQ(box.Min(), Vec3f(1));
    EXPECT_EQ(box.Max(), Vec3f(3));

    box.ExpandPoint(Vec3f(-10));
    EXPECT_EQ(box.Min(), Vec3f(-10));
    EXPECT_EQ(box.Max(), Vec3f(3));
}

TEST(box, union_box) {
    Box3 box1{ Vec3f(-10), Vec3f(5) };
    Box3 box2{ Vec3f(-5), Vec3f(10) };

    box1.UnionBox(box2);
    EXPECT_EQ(box1.Min(), Vec3f(-10));
    EXPECT_EQ(box1.Max(), Vec3f(10));
}

TEST(box, intersect_box) {
    Box3 box1{ Vec3f(-10), Vec3f(5) };
    Box3 box2{ Vec3f(-5), Vec3f(10) };

    box1.IntersectBox(box2);
    EXPECT_EQ(box1.Min(), Vec3f(-5));
    EXPECT_EQ(box1.Max(), Vec3f(5));
}

TEST(box, center_and_size) {
    Box3 box{ Vec3f(-10), Vec3f(5) };

    EXPECT_EQ(box.Center(), Vec3f(-2.5));
    EXPECT_EQ(box.Size(), Vec3f(15));
}

}  // namespace cave::math
