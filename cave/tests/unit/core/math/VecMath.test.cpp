#include "Vec.test.h"

namespace cave::math {

// add
TEST(vector_math, add) {
    {
        Vec4i vec1(1, 2, 3, 4);
        Vec4i vec2(5, 4, 3, 1);
        Vec4i result = vec1 + vec2;
        CHECK_VEC4(result, 6, 6, 6, 5);
    }
    {
        Vec3u vec1(1, 3, 4);
        Vec3u vec2(4, 3, 1);
        Vec3u result = vec1 + vec2;
        CHECK_VEC3(result, 5u, 6u, 5u);
    }
}

TEST(vector_math, add_scalar) {
    {
        Vec4i vec1(1, 2, 3, 4);
        Vec4i result = vec1 + 2;
        CHECK_VEC4(result, 3, 4, 5, 6);
    }
    {
        Vec3i vec1(1, 3, 4);
        Vec3i result = 1 + vec1;
        CHECK_VEC3(result, 2, 4, 5);
    }
}

TEST(vector_math, add_asign_scalar) {
    {
        Vec3i vec1(1, 3, 4);
        vec1 += 4;
        CHECK_VEC3(vec1, 5, 7, 8);
    }
    {
        Vec3i vec1(1, 3, 4);
        vec1 += -1;
        CHECK_VEC3(vec1, 0, 2, 3);
    }
}

TEST(vector_math, add_assign) {
    {
        Vec2i vec1(1, 2);
        Vec2i vec2(3, 4);
        Vec2i vec3(5, 6);
        Vec2i vec4(7, 8);
        vec1 += vec3;
        vec2 += vec4;
        CHECK_VEC2(vec1, 6, 8);
        CHECK_VEC2(vec2, 10, 12);
        CHECK_VEC2(vec3, 5, 6);
        CHECK_VEC2(vec4, 7, 8);
    }
    {
        Vec4i vec1(1, 2, 3, 4);
        Vec4i vec2(5, 4, 3, 1);
        vec1 += vec2;
        CHECK_VEC4(vec1, 6, 6, 6, 5);
    }
}

// sub
TEST(vector_math, sub) {
    {
        Vec4i vec1(5, 4, 3, 1);
        Vec4i vec2(1, 2, 3, 4);
        Vec4i result = vec1 - vec2;
        CHECK_VEC4(result, 4, 2, 0, -3);
    }
    {
        Vec3i vec1(1, 3, 4);
        Vec3i vec2(4, 3, 1);
        Vec3i result = vec1 - vec2;
        CHECK_VEC3(result, -3, 0, 3);
    }
}

TEST(vector_math, sub_scalar) {
    {
        Vec4i vec1(1, 2, 3, 4);
        Vec4i result = vec1 - (-2);
        CHECK_VEC4(result, 3, 4, 5, 6);
    }
    {
        Vec3f vec1(1.0f, 3.0f, 4.0f);
        Vec3f result = 1 - vec1;
        CHECK_VEC3(result, -0.0f, -2.0f, -3.0f);
    }
}

TEST(vector_math, sub_asign_scalar) {
    {
        Vec3i vec1(1, 3, 4);
        vec1 -= 4;
        CHECK_VEC3(vec1, -3, -1, 0);
    }
    {
        Vec3i vec1(1, 3, 4);
        vec1 -= 1;
        CHECK_VEC3(vec1, 0, 2, 3);
    }
}

TEST(vector_math, sub_assign) {
    {
        Vec2i vec1(1, 2);
        Vec2i vec2(3, 4);
        Vec2i vec3(5, 6);
        Vec2i vec4(7, 8);
        vec1 -= vec3;
        vec2 -= vec4;
        CHECK_VEC2(vec1, -4, -4);
        CHECK_VEC2(vec2, -4, -4);
        CHECK_VEC2(vec3, 5, 6);
        CHECK_VEC2(vec4, 7, 8);
    }
    {
        Vec4i vec1(1, 2, 3, 4);
        Vec4i vec2(5, 4, 3, 1);
        vec1 -= vec2;
        CHECK_VEC4(vec1, -4, -2, 0, 3);
    }
}

// mul
TEST(vector_math, mul) {
    {
        Vec4i vec1(5, 4, 3, 1);
        Vec4i vec2(1, 2, 3, 4);
        Vec4i result = vec1 * vec2;
        CHECK_VEC4(result, 5, 8, 9, 4);
    }
    {
        Vec3i vec1(1, 3, 4);
        Vec3i vec2(4, 3, 1);
        Vec3i result = vec1 * vec2;
        CHECK_VEC3(result, 4, 9, 4);
    }
}

TEST(vector_math, mul_scalar) {
    {
        Vec4i vec1(1, 2, 3, 4);
        Vec4i result = vec1 * 2;
        CHECK_VEC4(result, 2, 4, 6, 8);
    }
    {
        Vec3f vec1(1.0f, 3.0f, 4.0f);
        Vec3f result = 1 * vec1;
        CHECK_VEC3(result, 1.0f, 3.0f, 4.0f);
    }
}

TEST(vector_math, mul_asign_scalar) {
    {
        Vec3i vec1(1, 3, 4);
        vec1 *= 4;
        CHECK_VEC3(vec1, 4, 12, 16);
    }
    {
        Vec3i vec1(1, 3, 4);
        vec1 *= 9;
        CHECK_VEC3(vec1, 9, 27, 36);
    }
}

TEST(vector_math, mul_assign) {
    {
        Vec2i vec1(1, 2);
        Vec2i vec2(3, 4);
        Vec2i vec3(5, 6);
        Vec2i vec4(7, 8);
        vec1 *= vec3;
        vec2 *= vec4;
        CHECK_VEC2(vec1, 5, 12);
        CHECK_VEC2(vec2, 21, 32);
        CHECK_VEC2(vec3, 5, 6);
        CHECK_VEC2(vec4, 7, 8);
    }
    {
        Vec4i vec1(1, 2, 3, 4);
        Vec4i vec2(5, 4, 3, 1);
        vec1 *= vec2;
        CHECK_VEC4(vec1, 5, 8, 9, 4);
    }
}

// div
TEST(vector_math, div) {
    {
        Vec4i vec1(5, 4, 3, 1);
        Vec4i vec2(1, 2, 3, 4);
        Vec4i result = vec1 / vec2;
        CHECK_VEC4(result, 5, 2, 1, 0);
    }
    {
        Vec3i vec1(12, 12, 4);
        Vec3i vec2(4, 3, 2);
        Vec3i result = vec1 / vec2;
        CHECK_VEC3(result, 3, 4, 2);
    }
}

TEST(vector_math, div_scalar) {
    {
        Vec4f vec1(1, 2, 3, 4);
        Vec4f result = vec1 / 2.f;
        CHECK_VEC4(result, 0.5f, 1.f, 1.5f, 2.f);
    }
}

TEST(vector_math, div_asign_scalar) {
    {
        Vec3f vec1(40, 8, 4);
        vec1 /= 4.f;
        CHECK_VEC3(vec1, 10, 2, 1);
    }
    {
        Vec3f vec1(1, 3, 4);
        vec1 /= -1.f;
        CHECK_VEC3(vec1, -1, -3, -4);
    }
}

TEST(vector_math, div_assign) {
    {
        Vec2f vec1(21, 20);
        Vec2f vec2(8, 4);
        Vec2f vec3(7, 5);
        Vec2f vec4(2, 4);
        vec1 /= vec3;
        vec2 /= vec4;
        CHECK_VEC2(vec1, 3, 4);
        CHECK_VEC2(vec2, 4, 1);
        CHECK_VEC2(vec3, 7, 5);
        CHECK_VEC2(vec4, 2, 4);
    }
    {
        Vec4i vec1(6, 8, 3, 81);
        Vec4i vec2(5, 4, 3, 9);
        vec1 /= vec2;
        CHECK_VEC4(vec1, 1, 2, 1, 9);
    }
}

TEST(vector_math, lerp) {
    {
        const Vec2f a(4.0f, 1.0f);
        const Vec2f b(2.0f, 3.0f);
        auto vec = lerp(a, b, 0.0f);
        CHECK_VEC2(vec, 4, 1);
        vec = lerp(a, b, 1.0f);
        CHECK_VEC2(vec, 2, 3);
        vec = lerp(a, b, 0.1f);
        CHECK_VEC2(vec, 3.8f, 1.2f);
        CHECK_VEC2(a, 4, 1);
        CHECK_VEC2(b, 2, 3);
    }
    {
        const Vec4f a(4.0f, 20.0f, 12.0f, 4.0f);
        const Vec4f b(2.0f, 4.0f, 6.0f, 8.0f);
        auto vec = lerp(a, b, .5f);
        CHECK_VEC4(vec, 3, 12, 9, 6);
    }
}

TEST(vector_math, dot) {
    {
        const Vec2i a(4, 1);
        const Vec2i b(2, 3);
        EXPECT_EQ(dot(a, b), 11);
    }
    {
        const Vec4i a(1, 2, 3, 4);
        const Vec4i b(2, 4, 6, 8);
        EXPECT_EQ(dot(a, b), 60);
    }
}

TEST(vector_math, length) {
    {
        auto len = length(Vec2f(3, 4));
        EXPECT_FLOAT_EQ(len, 5.0f);
    }
    {
        auto len = length(Vec3f(3, 4, 5));
        EXPECT_FLOAT_EQ(len, 7.0710678f);
    }
    {
        auto len = length(Vec4f::One);
        EXPECT_FLOAT_EQ(len, 2.f);
    }
}

TEST(vector_math, normalize) {
    {
        Vec2f vec1(3, 4);
        auto vec2 = normalize(vec1);
        EXPECT_FLOAT_EQ(length(vec2), 1.0f);
        EXPECT_FLOAT_EQ(vec2.x, 3.0f / 5);
        EXPECT_FLOAT_EQ(vec2.y, 4.0f / 5);
    }
    {
        Vec3f vec1(1, 2, 2);
        auto vec2 = normalize(vec1);
        EXPECT_FLOAT_EQ(length(vec2), 1.0f);
        EXPECT_FLOAT_EQ(vec2.x, 1.0f / 3);
        EXPECT_FLOAT_EQ(vec2.y, 2.0f / 3);
        EXPECT_FLOAT_EQ(vec2.z, 2.0f / 3);
    }
    {
        Vec4f vec1(27, 36, 77, 122);
        auto vec2 = normalize(vec1);
        constexpr float err = 0.001f;
        EXPECT_FLOAT_EQ(length(vec2), 1.0f);
        EXPECT_NEAR(vec2.x, 27.f / 151, err);
        EXPECT_NEAR(vec2.y, 36.f / 151, err);
        EXPECT_NEAR(vec2.z, 77.f / 151, err);
        EXPECT_NEAR(vec2.w, 122.f / 151, err);
    }
}

TEST(vector_math, radians) {
    {
        constexpr float a = 90.0f;
        EXPECT_FLOAT_EQ(radians(a), halfPi());
    }
    {
        constexpr float a = 180.0f;
        EXPECT_FLOAT_EQ(radians(a), pi());
    }
    {
        constexpr float a = 360.0f;
        EXPECT_FLOAT_EQ(radians(a), twoPi());
    }
}

TEST(vector_math, degrees) {
    {
        constexpr float a = halfPi();
        EXPECT_FLOAT_EQ(degrees(a), 90.0f);
    }
    {
        constexpr float a = pi();
        EXPECT_FLOAT_EQ(degrees(a), 180.0f);
    }
    {
        constexpr float a = twoPi();
        EXPECT_FLOAT_EQ(degrees(a), 360.0f);
    }
}

TEST(vector_math, cross) {
    constexpr Vec3f a(1, 2, 3);
    constexpr Vec3f b(5, 6, 0);
    constexpr auto c = cross(a, b);
    EXPECT_FLOAT_EQ(c.x, -18);
    EXPECT_FLOAT_EQ(c.y, 15);
    EXPECT_FLOAT_EQ(c.z, -4);
}

TEST(vector_math, cross_parallel_vector) {
    constexpr Vec3f a(1, 2, 3);
    constexpr auto c = cross(a, 2 * a);
    EXPECT_FLOAT_EQ(c.x, 0);
    EXPECT_FLOAT_EQ(c.y, 0);
    EXPECT_FLOAT_EQ(c.z, 0);
}

// @TODO: finish the tests
TEST(vector_math, min) {
    {
        constexpr int a = 10;
        constexpr int b = 3;
        EXPECT_EQ(min(a, b), 3);
    }
    {
        constexpr Vec2f a(1, 3);
        constexpr Vec2f b(-4, 5);
        constexpr auto v = min(a, b);
        EXPECT_EQ(v.x, -4);
        EXPECT_EQ(v.y, 3);
    }
}

TEST(vector_math, max) {
    {
        constexpr int a = 10;
        constexpr int b = 3;
        EXPECT_EQ(max(a, b), 10);
    }
}

}  // namespace cave::math
