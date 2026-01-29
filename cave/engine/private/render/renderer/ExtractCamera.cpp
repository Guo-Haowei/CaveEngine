#include "ExtractCamera.h"

#include "engine/private/runtime/scene/CameraComponent.h"
#include "engine/private/runtime/scene/ISceneRegistry.h"

namespace cave::render {

void ExtractCamera(const CameraComponent& p_camera,
                   bool p_is_opengl,
                   CameraParams& p_out_cam) {
    DEV_ASSERT(!p_camera.IsDirty());  // make sure camera has updated

    CameraParams& c = p_out_cam;  // alias

    c.vp_w = static_cast<float>(p_camera.GetWidth());
    c.vp_h = static_cast<float>(p_camera.GetHeight());
    c.aspect = p_camera.GetAspect();
    c.fovy = p_camera.GetFovy();

    c.view = p_camera.GetViewMatrix();
    c.proj_culling = p_camera.GetProjectionMatrix();

    auto reverse_z = [](math::Matrix4x4f& p_perspective) {
        constexpr math::Matrix4x4f matrix{ 1.0f, 0.0f, 0.0f, 0.0f,
                                           0.0f, 1.0f, 0.0f, 0.0f,
                                           0.0f, 0.0f, -1.0f, 0.0f,
                                           0.0f, 0.0f, 1.0f, 1.0f };
        p_perspective = matrix * p_perspective;
    };
    auto normalize_unit_range = [](math::Matrix4x4f& p_perspective) {
        constexpr math::Matrix4x4f matrix{ 1.0f, 0.0f, 0.0f, 0.0f,
                                           0.0f, 1.0f, 0.0f, 0.0f,
                                           0.0f, 0.0f, 0.5f, 0.0f,
                                           0.0f, 0.0f, 0.5f, 1.0f };
        p_perspective = matrix * p_perspective;
    };

    // https://tomhultonharrop.com/mathematics/graphics/2023/08/06/reverse-z.html
    if (p_is_opengl) {
        // since we use opengl matrix for frustum culling,
        // we can use the same matrix for rendering
        c.proj_rendering = c.proj_culling;
        normalize_unit_range(c.proj_rendering);
        reverse_z(c.proj_rendering);
    } else {
        c.proj_rendering = p_camera.CalcProjection();
        reverse_z(c.proj_rendering);
    }

    c.front = p_camera.GetFront();
    c.right = p_camera.GetRight();
    c.up = p_camera.GetUp();

    c.position = p_camera.GetPosition();
}

}  // namespace cave::render
