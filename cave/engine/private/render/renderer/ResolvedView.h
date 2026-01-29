#pragma once
#include "cave/core/math/Matrix.h"
#include "cave/render/ViewDesc.h"

// @TODO: move to public
#include "engine/private/core/math/Frustum.h"

namespace cave {
class Scene;
}  // namespace cave

namespace cave::render {

struct ResolvedView {
    math::Matrix4x4f view;
    math::Matrix4x4f proj_rendering;
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
};

ResolvedView ResolveView(const ViewDesc& p_view, const Scene* p_scene, bool p_is_opengl);

}  // namespace cave::render