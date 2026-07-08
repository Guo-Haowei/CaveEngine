// =============================================================================
// File: cave/core/math/Vec.h
// =============================================================================
#pragma once
#include "impl/Vec2.h"
#include "impl/Vec3.h"
#include "impl/Vec4.h"
#include "impl/VecMath.h"

namespace cave::math {

using Vec2i = Vec<int, 2>;
using Vec3i = Vec<int, 3>;
using Vec4i = Vec<int, 4>;
using Vec2u = Vec<uint32_t, 2>;
using Vec3u = Vec<uint32_t, 3>;
using Vec4u = Vec<uint32_t, 4>;
using Vec2f = Vec<float, 2>;
using Vec3f = Vec<float, 3>;
using Vec4f = Vec<float, 4>;

static_assert(sizeof(Vec2i) == 8);
static_assert(sizeof(Vec3i) == 12);
static_assert(sizeof(Vec4i) == 16);
static_assert(sizeof(Vec2u) == 8);
static_assert(sizeof(Vec3u) == 12);
static_assert(sizeof(Vec4u) == 16);
static_assert(sizeof(Vec2f) == 8);
static_assert(sizeof(Vec3f) == 12);
static_assert(sizeof(Vec4f) == 16);

static_assert(alignof(Vec4f) == 16);

}  // namespace cave::math
