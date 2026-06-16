// =============================================================================
// File: cave/core/math/Vector.h
// =============================================================================
#pragma once
#include "impl/Vector2.h"
#include "impl/Vector3.h"
#include "impl/Vector4.h"
#include "impl/VectorMath.h"

namespace cave::math {

using Vec2i = Vector<int, 2>;
using Vec3i = Vector<int, 3>;
using Vec4i = Vector<int, 4>;
using Vec2u = Vector<uint32_t, 2>;
using Vec3u = Vector<uint32_t, 3>;
using Vec4u = Vector<uint32_t, 4>;
using Vec2f = Vector<float, 2>;
using Vec3f = Vector<float, 3>;
using Vec4f = Vector<float, 4>;

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
