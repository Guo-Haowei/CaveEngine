// =============================================================================
// File: cave/render/PrimitiveData.h
// =============================================================================
#pragma once
#include "cave/core/containers/Containers.h"
#include "cave/core/math/Vec.h"

namespace cave::render {

// @TODO: this doesn't have to be in render module
struct PrimVert {
    math::Vec3f pos;
    math::Vec2f uv;
    math::Vec4f color;
};

}  // namespace cave::render
