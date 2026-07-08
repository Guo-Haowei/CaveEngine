#include "Vec.test.h"

namespace cave::math {

TEST(Vec, ConstructVec2) {
    CHECK_VEC2(Vec2u::Zero, 0u, 0u);
    CHECK_VEC2(Vec2u::UnitX, 1u, 0u);
    CHECK_VEC2(Vec2u::UnitY, 0u, 1u);
    CHECK_VEC2(Vec2u::One, 1u, 1u);
    {
        Vec2f vec(1.0, 2.0f);
        CHECK_VEC2(vec, 1, 2);
    }
}

TEST(Vec, ConstructVec3) {
    CHECK_VEC3(Vec3u::Zero, 0u, 0u, 0u);
    CHECK_VEC3(Vec3u::UnitX, 1u, 0u, 0u);
    CHECK_VEC3(Vec3u::UnitZ, 0u, 0u, 1u);
    CHECK_VEC3(Vec3u::One, 1u, 1u, 1u);
    {
        Vec3f vec(1, 2, 3);
        CHECK_VEC3(vec, 1, 2, 3);
    }
    {
        Vec3f vec(Vec2f(1, 2), 3);
        CHECK_VEC3(vec, 1, 2, 3);
    }
}

TEST(Vec, ConstructVec4) {
    CHECK_VEC4(Vec4f::Zero, 0, 0, 0, 0);
    CHECK_VEC4(Vec4f::One, 1, 1, 1, 1);
    CHECK_VEC4(Vec4f::UnitW, 0, 0, 0, 1);
    CHECK_VEC4(Vec4f::UnitY, 0, 1, 0, 0);
    {
        Vec4f vec(Vec3f(1.0f, 2.0f, 3.0f), 4.0f);
        CHECK_VEC4(vec, 1, 2, 3, 4);
    }
    {
        Vec4i vec(Vec2i(1, 2), 3, 4);
        CHECK_VEC4(vec, 1, 2, 3, 4);
    }
    {
        Vec4i vec(Vec2i(1, 2), Vec2i(3, 4));
        CHECK_VEC4(vec, 1, 2, 3, 4);
    }
}

TEST(Vec, ConstructWithDifferentType) {
    {
        int a = 1;
        int b = 2;
        Vec2f vec(a, b);
        CHECK_VEC2(vec, 1, 2);
    }
    {
        float a = 1.4f;
        float b = 2.2f;
        float c = -3.3f;
        Vec3i vec(a, b, c);
        CHECK_VEC3(vec, 1, 2, -3);
    }
    {
        int a = 5;
        int b = 2;
        int c = 3;
        int d = 7;
        Vec4f vec(a, b, c, d);
        CHECK_VEC4(vec, 5, 2, 3, 7);
    }
}

TEST(Vec, Swizzle) {
    Vec4f vec = Vec4f::UnitY;
    vec[2] = 1;

    Vec2f a = vec.yz;
    CHECK_VEC2(a, 1, 1);
}

}  // namespace cave::math
