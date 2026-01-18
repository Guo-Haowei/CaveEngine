#include "camera_controller.h"

#include "engine/math/angle.h"
#include "engine/runtime/input_manager.h"
#include "engine/scene/camera_component.h"
#include "engine/scene/transform_component.h"

namespace cave {

void CameraController2DEditor::Update(const CameraInputState& p_state) {
    DEV_ASSERT(0);

    CameraComponent  p_camera;
    TransformComponent  p_transform;
    const bool moved = p_state.move.x || p_state.move.y;
    if (moved) {
        p_transform.Translate(Vector3f(p_state.move.x, p_state.move.y, 0.0f));
    }

    if (p_state.zoom_delta != 0.0f) {
        float ortho_height = p_camera.GetOrthoHeight() + 4.0f * p_state.zoom_delta;
        ortho_height = glm::clamp(ortho_height, 0.1f, 100.0f);
        p_camera.SetOrthoHeight(ortho_height);
    }
}

void CameraControllerFPS::Update(const CameraInputState& p_state) {
    CameraComponent p_camera;
    TransformComponent p_transform;

    const bool moved = p_state.move.x || p_state.move.y || p_state.move.z || p_state.zoom_delta != 0.0f;
    if (moved) {
        const float dx = p_state.move.x;
        const float dy = p_state.move.y;

        float dz = p_state.move.z;
        const float scroll_z = m_scrollSpeed * p_state.zoom_delta;
        if (glm::abs(scroll_z) > glm::abs(dz)) {
            dz = scroll_z;
        }

        Vector3f position = p_transform.GetTranslation();

        if (dx || dz) {
            Vector3f delta = (m_moveSpeed * dz) * p_camera.GetFront() + (m_moveSpeed * dx) * p_camera.GetRight();
            p_transform.Translate(delta);
        }
        if (dy) {
            p_transform.Translate(Vector3f(0.0f, m_moveSpeed * dy, 0.0f));
        }
    }

    auto rotate_camera = [&]() {
        float rotate_x = 0.0f;
        float rotate_y = 0.0f;

        Vector2f movement = p_state.rotation;
        movement = m_rotateSpeed * movement;
        if (glm::abs(movement.x) > glm::abs(movement.y)) {
            rotate_y = movement.x;
        } else {
            rotate_x = movement.y;
        }

        // @TODO: DPI
#if 0
        if (rotate_y) {
            p_camera.m_yaw += Degree(rotate_y);
        }

        if (rotate_x) {
            p_camera.m_pitch += Degree(rotate_x);
            p_camera.m_pitch.Clamp(-85.0f, 85.0f);
        }
#endif

        return rotate_x != 0.0f || rotate_y != 0.0f;
    };

    if (moved || rotate_camera()) {
        p_camera.SetDirtyFlag();
    }
}

}  // namespace cave
