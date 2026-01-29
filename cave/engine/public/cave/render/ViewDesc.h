// =============================================================================
// File: public/cave/render/ViewDesc.h
// =============================================================================
#pragma once
#include "cave/core/ids/SceneId.h"
#include "cave/core/math/Rect.h"

#include "cave/render/CameraParams.h"

namespace cave::render {

using RenderTargetId = uint32_t;

struct RenderOutputDesc {
    RenderTargetId color{};
    RenderTargetId depth{};
    bool clear_color = true;
    bool clear_depth = true;
};

struct ViewDesc {
    SceneId scene_id;
    CameraParams camera;
    math::IntRect viewport_pixel;
    RenderOutputDesc output;
    // missing: which render target info it draws to
};

}  // namespace cave::render