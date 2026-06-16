#pragma once
#include "cave/core/math/Vector.h"

#define CHECK_VEC2(VEC, a, b)  \
    {                          \
        EXPECT_EQ((VEC).x, a); \
        EXPECT_EQ((VEC).y, b); \
    }
#define CHECK_VEC3(VEC, a, b, c) \
    {                            \
        EXPECT_EQ((VEC).x, a);   \
        EXPECT_EQ((VEC).y, b);   \
        EXPECT_EQ((VEC).z, c);   \
    }
#define CHECK_VEC4(VEC, a, b, c, d) \
    {                               \
        EXPECT_EQ((VEC).x, a);      \
        EXPECT_EQ((VEC).y, b);      \
        EXPECT_EQ((VEC).z, c);      \
        EXPECT_EQ((VEC).w, d);      \
    }

namespace cave::math {

static_assert(sizeof(Vec2f) == 8);
static_assert(sizeof(Vec3f) == 12);
static_assert(sizeof(Vec4f) == 16);
static_assert(sizeof(Vec2i) == 8);
static_assert(sizeof(Vec3i) == 12);
static_assert(sizeof(Vec4i) == 16);
static_assert(sizeof(Vec2u) == 8);
static_assert(sizeof(Vec3u) == 12);
static_assert(sizeof(Vec4u) == 16);

}  // namespace cave::math