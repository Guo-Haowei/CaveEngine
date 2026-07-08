#include "Vec.test.h"

namespace cave::math {

TEST(Swizzle2, vector2_swizzle_read) {
    const Vec2i vec2(2, 5);
    {
        Vec2i vec = vec2.xx;
        EXPECT_EQ(vec.x, 2);
        EXPECT_EQ(vec.y, 2);
    }
    {
        Vec2i vec = vec2.xy;
        EXPECT_EQ(vec.x, 2);
        EXPECT_EQ(vec.y, 5);
    }
    {
        Vec2i vec = vec2.yx;
        EXPECT_EQ(vec.x, 5);
        EXPECT_EQ(vec.y, 2);
    }
    {
        Vec2i vec = vec2.yy;
        EXPECT_EQ(vec.x, 5);
        EXPECT_EQ(vec.y, 5);
    }
}

TEST(Swizzle2, vector3_swizzle_read) {
    const Vec3i vec3(7, 8, 9);
    {
        Vec2i vec = vec3.xx;
        EXPECT_EQ(vec.x, 7);
        EXPECT_EQ(vec.y, 7);
    }
    {
        Vec2i vec = vec3.xy;
        EXPECT_EQ(vec.x, 7);
        EXPECT_EQ(vec.y, 8);
    }
    {
        Vec2i vec = vec3.xz;
        EXPECT_EQ(vec.x, 7);
        EXPECT_EQ(vec.y, 9);
    }
    {
        Vec2i vec = vec3.yx;
        EXPECT_EQ(vec.x, 8);
        EXPECT_EQ(vec.y, 7);
    }
    {
        Vec2i vec = vec3.yy;
        EXPECT_EQ(vec.x, 8);
        EXPECT_EQ(vec.y, 8);
    }
    {
        Vec2i vec = vec3.yz;
        EXPECT_EQ(vec.x, 8);
        EXPECT_EQ(vec.y, 9);
    }
    {
        Vec2i vec = vec3.zx;
        EXPECT_EQ(vec.x, 9);
        EXPECT_EQ(vec.y, 7);
    }
    {
        Vec2i vec = vec3.zy;
        EXPECT_EQ(vec.x, 9);
        EXPECT_EQ(vec.y, 8);
    }
    {
        Vec2i vec = vec3.zz;
        EXPECT_EQ(vec.x, 9);
        EXPECT_EQ(vec.y, 9);
    }
}

TEST(Swizzle2, vector4_swizzle_read) {
    const Vec4i vec4(7, 8, 9, 10);
    {
        Vec2i vec = vec4.xx;
        EXPECT_EQ(vec.x, 7);
        EXPECT_EQ(vec.y, 7);
    }
    {
        Vec2i vec = vec4.xy;
        EXPECT_EQ(vec.x, 7);
        EXPECT_EQ(vec.y, 8);
    }
    {
        Vec2i vec = vec4.xz;
        EXPECT_EQ(vec.x, 7);
        EXPECT_EQ(vec.y, 9);
    }
    {
        Vec2i vec = vec4.xw;
        EXPECT_EQ(vec.x, 7);
        EXPECT_EQ(vec.y, 10);
    }
    {
        Vec2i vec = vec4.yx;
        EXPECT_EQ(vec.x, 8);
        EXPECT_EQ(vec.y, 7);
    }
    {
        Vec2i vec = vec4.yy;
        EXPECT_EQ(vec.x, 8);
        EXPECT_EQ(vec.y, 8);
    }
    {
        Vec2i vec = vec4.yz;
        EXPECT_EQ(vec.x, 8);
        EXPECT_EQ(vec.y, 9);
    }
    {
        Vec2i vec = vec4.yw;
        EXPECT_EQ(vec.x, 8);
        EXPECT_EQ(vec.y, 10);
    }
    {
        Vec2i vec = vec4.zx;
        EXPECT_EQ(vec.x, 9);
        EXPECT_EQ(vec.y, 7);
    }
    {
        Vec2i vec = vec4.zy;
        EXPECT_EQ(vec.x, 9);
        EXPECT_EQ(vec.y, 8);
    }
    {
        Vec2i vec = vec4.zz;
        EXPECT_EQ(vec.x, 9);
        EXPECT_EQ(vec.y, 9);
    }
    {
        Vec2i vec = vec4.zw;
        EXPECT_EQ(vec.x, 9);
        EXPECT_EQ(vec.y, 10);
    }
    {
        Vec2i vec = vec4.wx;
        EXPECT_EQ(vec.x, 10);
        EXPECT_EQ(vec.y, 7);
    }
    {
        Vec2i vec = vec4.wy;
        EXPECT_EQ(vec.x, 10);
        EXPECT_EQ(vec.y, 8);
    }
    {
        Vec2i vec = vec4.wz;
        EXPECT_EQ(vec.x, 10);
        EXPECT_EQ(vec.y, 9);
    }
    {
        Vec2i vec = vec4.ww;
        EXPECT_EQ(vec.x, 10);
        EXPECT_EQ(vec.y, 10);
    }
}

TEST(Swizzle2, vector2_swizzle_write) {
    {
        Vec2i vec(2, 5);
        vec.yx = Vec2i(7, 6);
        EXPECT_EQ(vec.x, 6);
        EXPECT_EQ(vec.y, 7);
    }
    {
        Vec2i vec;
        vec.xy = Vec2i(7, 6);
        EXPECT_EQ(vec.x, 7);
        EXPECT_EQ(vec.y, 6);
    }
}

TEST(Swizzle2, vector3_swizzle_write) {
    {
        Vec3i vec(0);
        vec.xy = Vec2i(7, 6);
        EXPECT_EQ(vec.x, 7);
        EXPECT_EQ(vec.y, 6);
        EXPECT_EQ(vec.z, 0);
    }
    {
        Vec3i vec(0);
        vec.xz = Vec2i(7, 6);
        EXPECT_EQ(vec.x, 7);
        EXPECT_EQ(vec.y, 0);
        EXPECT_EQ(vec.z, 6);
    }
    {
        Vec3i vec(0);
        vec.yx = Vec2i(7, 6);
        EXPECT_EQ(vec.x, 6);
        EXPECT_EQ(vec.y, 7);
        EXPECT_EQ(vec.z, 0);
    }
    {
        Vec3i vec(0);
        vec.yx = Vec2i(7, 6);
        EXPECT_EQ(vec.x, 6);
        EXPECT_EQ(vec.y, 7);
        EXPECT_EQ(vec.z, 0);
    }
    {
        Vec3i vec(0);
        vec.yz = Vec2i(7, 6);
        EXPECT_EQ(vec.x, 0);
        EXPECT_EQ(vec.y, 7);
        EXPECT_EQ(vec.z, 6);
    }
    {
        Vec3i vec(0);
        vec.zx = Vec2i(7, 6);
        EXPECT_EQ(vec.x, 6);
        EXPECT_EQ(vec.y, 0);
        EXPECT_EQ(vec.z, 7);
    }
    {
        Vec3i vec(0);
        vec.zy = Vec2i(7, 6);
        EXPECT_EQ(vec.x, 0);
        EXPECT_EQ(vec.y, 6);
        EXPECT_EQ(vec.z, 7);
    }
}

}  // namespace cave::math
