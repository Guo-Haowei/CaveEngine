#include "SceneView.h"

#include "engine/private/runtime/scene/CameraComponent.h"
#include "engine/private/runtime/scene/ISceneRegistry.h"

namespace cave {

void ViewInfo::FromCamera(const CameraComponent& p_camera,
                          ViewInfo& p_out_view_info,
                          bool p_is_opengl) {

    p_out_view_info.sceen_width = static_cast<float>(p_camera.GetWidth());
    p_out_view_info.sceen_height = static_cast<float>(p_camera.GetHeight());
    p_out_view_info.aspect_ratio = p_out_view_info.sceen_width / p_out_view_info.sceen_height;
    p_out_view_info.fovy = p_camera.GetFovy();

    p_out_view_info.view = p_camera.GetViewMatrix();
    p_out_view_info.projection_frustum = p_camera.GetProjectionMatrix();

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
        p_out_view_info.projection_rendering = p_out_view_info.projection_frustum;
        normalize_unit_range(p_out_view_info.projection_rendering);
        reverse_z(p_out_view_info.projection_rendering);
    } else {
        p_out_view_info.projection_rendering = p_camera.CalcProjection();
        reverse_z(p_out_view_info.projection_rendering);
    }

    p_out_view_info.front = p_camera.GetFront();
    p_out_view_info.right = p_camera.GetRight();
    p_out_view_info.up = p_camera.GetUp();

    p_out_view_info.position = p_camera.GetPosition();
}

Scene* SceneView::ResolveScene() {
    if (DEV_VERIFY(scene_manager)) {
        return scene_manager->Resolve(scene_id);
    }
    return nullptr;
}

}  // namespace cave
