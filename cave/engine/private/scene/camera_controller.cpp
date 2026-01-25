#include "camera_controller.h"

#include "engine/math/angle.h"
#include "engine/scene/scene.h"

namespace cave {

void CameraController2DEditor::Update(const CameraInputState& p_state) {
    CameraComponent* camera = m_scene->GetComponent<CameraComponent>(m_cam);
    TransformComponent* transform = m_scene->GetComponent<TransformComponent>(m_cam);

    const bool moved = p_state.move.x || p_state.move.y;
    if (moved) {
        transform->Translate(Vector3f(p_state.move.x, p_state.move.y, 0.0f));
    }

    if (p_state.zoom_delta != 0.0f) {
        float ortho_height = camera->GetOrthoHeight() + 4.0f * p_state.zoom_delta;
        ortho_height = glm::clamp(ortho_height, 0.1f, 100.0f);
        camera->SetOrthoHeight(ortho_height);
    }
}

void CameraControllerFPS::Update(const CameraInputState& p_state) {
    CameraComponent* camera = m_scene->GetComponent<CameraComponent>(m_cam);
    TransformComponent* rotation_x = m_scene->GetComponent<TransformComponent>(m_cam);
    TransformComponent* rotation_y = m_scene->GetComponent<TransformComponent>(m_cam_y);
    TransformComponent* root = m_scene->GetComponent<TransformComponent>(m_cam_root);
    DEV_ASSERT(camera && rotation_x && rotation_y && root);

    const bool moved = p_state.move.x || p_state.move.y || p_state.move.z || p_state.zoom_delta != 0.0f;
    if (moved) {
        const float dx = p_state.move.x;
        const float dy = p_state.move.y;

        float dz = p_state.move.z;
        const float scroll_z = m_scroll_speed * p_state.zoom_delta;
        if (glm::abs(scroll_z) > glm::abs(dz)) {
            dz = scroll_z;
        }

        Vector3f position = root->GetTranslation();

        if (dx || dz) {
            Vector3f delta = (m_move_speed * dz) * camera->GetFront() + (m_move_speed * dx) * camera->GetRight();
            root->Translate(delta);
        }
        if (dy) {
            root->Translate(Vector3f(0.0f, m_move_speed * dy, 0.0f));
        }
    }

    auto rotate_camera = [&]() {
        float rotate_x = 0.0f;
        float rotate_y = 0.0f;

        Vector2f movement = p_state.rotation;
        movement = m_rotate_speed * movement;
        if (glm::abs(movement.x) > glm::abs(movement.y)) {
            rotate_y = movement.x;
        } else {
            rotate_x = movement.y;
        }

        if (rotate_y) {
            rotation_y->RotateY(Degree(-rotate_y));
        }

        if (rotate_x) {
            rotation_x->RotateX(Degree(rotate_x));
        }

        return rotate_x != 0.0f || rotate_y != 0.0f;
    };

    if (moved || rotate_camera()) {
        camera->SetDirtyFlag();
    }
}

}  // namespace cave
