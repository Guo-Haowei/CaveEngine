#pragma once
#include "cave/core/math/Frustum.h"
#include "cave/render/ViewDesc.h"

// clang-format off
namespace cave { class Scene; }
// clang-format on

namespace cave::render {

struct ResolvedView {
    math::Matrix4x4f view;
    math::Matrix4x4f proj;
    math::Matrix4x4f view_inv;
    math::Matrix4x4f proj_inv;
    math::Frustum frustum;

    math::Vector3f position;
    math::Vector3f up;
    math::Vector3f front;
    math::Vector3f right;

    float vp_w;
    float vp_h;
    float aspect;
    float fovy;

    SceneId scene_id;
    Scene* scene{ nullptr };

    ViewHighlight highlight;
    GpuTextureId output;
};

}  // namespace cave::render