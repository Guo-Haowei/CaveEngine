#pragma once
#include "cave/core/math/Frustum.h"
#include "cave/runtime/view/ViewDesc.h"

namespace cave {

class Scene;

struct CameraParams {
    math::Matrix4x4f view;
    math::Matrix4x4f proj;
    math::Matrix4x4f view_inv;
    math::Matrix4x4f proj_inv;
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