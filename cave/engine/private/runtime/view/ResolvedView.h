#pragma once
#include "cave/core/math/Frustum.h"
#include "cave/runtime/view/ViewDesc.h"

namespace cave {

class Scene;

struct CameraParams {
    math::Mat4f view;
    math::Mat4f proj;
    math::Mat4f view_inv;
    math::Mat4f proj_inv;
};

struct ResolvedView {
    ViewId view_id;
    CameraParams cam;
    math::Frustum frustum;
    math::IntRect viewport_px;

    float fovy_rad;
    SceneId scene_id;
    Scene* scene{ nullptr };

    ViewHighlight highlight;
    GpuTextureId output;
};

}  // namespace cave