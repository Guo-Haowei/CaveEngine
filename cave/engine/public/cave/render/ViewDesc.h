// =============================================================================
// File: public/cave/render/ViewDesc.h
// =============================================================================
#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/math/Rect.h"

namespace cave::render {

using RenderTargetId = uint32_t;

struct ViewOutputDesc {
    RenderTargetId color{};
    RenderTargetId depth{};
    bool clear_color = true;
    bool clear_depth = true;
};

struct ViewDesc {
    SceneId scene_id;
    ecs::Entity camera;
    // CameraParams camera;
    math::IntRect viewport_pixel;
    ViewOutputDesc output;
    // missing: which render target info it draws to
};

}  // namespace cave::render