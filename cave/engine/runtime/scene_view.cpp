#include "scene_view.h"

#include "engine/scene/camera_component.h"

namespace cave {

void ViewInfo::FromCamera(const CameraComponent& p_camera,
                          ViewInfo& p_out_view_info,
                          bool p_is_opengl) {

    // @TODO: refactor this part
    p_out_view_info.sceenWidth = static_cast<float>(p_camera.GetWidth());
    p_out_view_info.sceenHeight = static_cast<float>(p_camera.GetHeight());
    p_out_view_info.aspectRatio = p_out_view_info.sceenWidth / p_out_view_info.sceenHeight;
    p_out_view_info.fovy = p_camera.GetFovy();

    p_out_view_info.viewMatrix = p_camera.GetViewMatrix();
    p_out_view_info.projectionMatrixFrustum = p_camera.GetProjectionMatrix();

    p_out_view_info.sceenWidth = static_cast<float>(p_camera.GetWidth());
    p_out_view_info.sceenHeight = static_cast<float>(p_camera.GetHeight());
    p_out_view_info.aspectRatio = p_out_view_info.sceenWidth / p_out_view_info.sceenHeight;
    p_out_view_info.fovy = p_camera.GetFovy();

    p_out_view_info.viewMatrix = p_camera.GetViewMatrix();
    p_out_view_info.projectionMatrixFrustum = p_camera.GetProjectionMatrix();

    auto reverse_z = [](Matrix4x4f& p_perspective) {
        constexpr Matrix4x4f matrix{ 1.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 1.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, -1.0f, 0.0f,
                                     0.0f, 0.0f, 1.0f, 1.0f };
        p_perspective = matrix * p_perspective;
    };
    auto normalize_unit_range = [](Matrix4x4f& p_perspective) {
        constexpr Matrix4x4f matrix{ 1.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 1.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 0.5f, 0.0f,
                                     0.0f, 0.0f, 0.5f, 1.0f };
        p_perspective = matrix * p_perspective;
    };

    // https://tomhultonharrop.com/mathematics/graphics/2023/08/06/reverse-z.html
    if (p_is_opengl) {
        // since we use opengl matrix for frustum culling,
        // we can use the same matrix for rendering
        p_out_view_info.projectionMatrixRendering = p_out_view_info.projectionMatrixFrustum;
        normalize_unit_range(p_out_view_info.projectionMatrixRendering);
        reverse_z(p_out_view_info.projectionMatrixRendering);
    } else {
        p_out_view_info.projectionMatrixRendering = p_camera.CalcProjection();
        reverse_z(p_out_view_info.projectionMatrixRendering);
    }
    p_out_view_info.position = p_camera.GetPosition();

    p_out_view_info.front = p_camera.GetFront();
    p_out_view_info.right = p_camera.GetRight();
    p_out_view_info.up = cross(p_out_view_info.front, p_out_view_info.right);
}

}  // namespace cave
