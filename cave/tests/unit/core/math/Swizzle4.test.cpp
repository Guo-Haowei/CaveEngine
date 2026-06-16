#include "vector.test.h"

namespace cave::math {

TEST(Swizzle4, vector4_swizzle4_read) {
    {
        Vec4i vec = Vec4i(10, 14, 6, 2).wzyx;
        CHECK_VEC4(vec, 2, 6, 14, 10);
    }
    {
        Vec4i vec = Vec4i(10, 14, 6, 2).xxxx;
        CHECK_VEC4(vec, 10, 10, 10, 10);
    }
    {
        Vec4i vec = Vec4i(10, 14, 6, 2).yyww;
        CHECK_VEC4(vec, 14, 14, 2, 2);
    }
    {
        Vec4i vec = Vec4i(10, 14, 6, 2).xyzz;
        CHECK_VEC4(vec, 10, 14, 6, 6);
    }
}

TEST(Swizzle4, vector4_swizzle4_write) {
    {
        Vec4i vec;
        vec.xyzw = Vec4i(7, 6, 8, 9);
        CHECK_VEC4(vec, 7, 6, 8, 9);
    }
    {
        Vec4i vec;
        vec.wzyx = Vec4i(6, 7, 8, 9);
        CHECK_VEC4(vec, 9, 8, 7, 6);
    }
    {
        Vec4i vec;
        vec.wyzx = Vec4i(6, 7, 8, 9);
        CHECK_VEC4(vec, 9, 7, 8, 6);
    }
    {
        Vec4i vec;
        vec.xzyw = Vec4i(6, 7, 8, 9);
        CHECK_VEC4(vec, 6, 8, 7, 9);
    }
}

}  // namespace cave::math
