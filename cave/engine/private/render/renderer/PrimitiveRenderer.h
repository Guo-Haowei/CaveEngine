#pragma once
#include "cave/core/math/Matrix.h"
#include "cave/render/PrimitiveData.h"

namespace cave::render {

struct PrimitiveRenderParams {
    math::Mat4f view_proj;
    math::Vec2f viewport;
    bool depth_test = false;
    bool depth_write = false;
};

}  // namespace cave::render
