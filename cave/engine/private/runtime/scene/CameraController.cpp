#include "CameraController.h"

#include "cave/core/math/Angle.h"
#include "cave/runtime/scene/CameraComponent.h"

namespace cave {

using math::Matrix4x4f;
using math::Vector3f;

CameraController2DEditor::CameraController2DEditor(CameraComponent& p_camera,
                                                   TransformComponent& p_tranform)
    : m_camera(p_camera)
    , m_root(p_tranform) {
}

void CameraController2DEditor::Update(const CameraInputState& p_state) {
    const bool moved = p_state.move.x || p_state.move.y;

    bool need_update = false;
    if (moved) {
        constexpr float speed = 1.0f;
        m_root.Translate(math::Vector3f(p_state.move.x * speed,
                                        p_state.move.y * speed,
                                        0.0f));
        need_update = true;
    }

    if (p_state.zoom_delta != 0.0f) {
        float ortho_height = m_camera.GetOrthoHeight() + 8.0f * p_state.zoom_delta;
        ortho_height = glm::clamp(ortho_height, 0.1f, 100.0f);
        m_camera.SetOrthoHeight(ortho_height);
        need_update = true;
    }
    if (need_update) {
        m_camera.SetDirty();
        m_root.UpdateTransform();
        m_camera.Update(m_root.GetWorldMatrix());
    }
}

CameraControllerFPS::CameraControllerFPS(CameraComponent& p_camera,
                                         TransformComponent& p_tranform)
    : m_camera(p_camera)
    , m_root(p_tranform) {
}

void CameraControllerFPS::Update(const CameraInputState& p_state) {
    const bool moved = p_state.move.x || p_state.move.y || p_state.move.z || p_state.zoom_delta != 0.0f;
    if (moved) {
        const float dx = p_state.move.x;
        const float dy = p_state.move.y;

        float dz = p_state.move.z;
        const float scroll_z = m_scroll_speed * p_state.zoom_delta;
        if (glm::abs(scroll_z) > glm::abs(dz)) {
            dz = scroll_z;
        }

        if (dx || dz) {
            Vector3f delta = (m_move_speed * dz) * m_camera.GetFront() + (m_move_speed * dx) * m_camera.GetRight();
            m_root.Translate(delta);
        }
        if (dy) {
            m_root.Translate(Vector3f(0.0f, m_move_speed * dy, 0.0f));
        }
    }

    auto rotate_camera = [&]() {
        float rotate_x = 0.0f;
        float rotate_y = 0.0f;

        math::Vector2f movement = p_state.rotation;
        movement = m_rotate_speed * movement;
        if (glm::abs(movement.x) > glm::abs(movement.y)) {
            rotate_y = movement.x;
        } else {
            rotate_x = movement.y;
        }

        if (rotate_y) {
            m_root.RotateY(math::Degree(-rotate_y));
        }

        if (rotate_x) {
            m_pitch += rotate_x;
            m_pitch = math::clamp(m_pitch, -80.0f, 80.0f);
            m_camera.SetDirty();
        }

        return rotate_x != 0.0f || rotate_y != 0.0f;
    };

    if (moved || rotate_camera()) {
        m_camera.SetDirty();
    }
    if (m_root.IsDirty()) {
        m_camera.SetDirty();
    }

    m_root.UpdateTransform();
    math::Matrix4x4f rotation = glm::rotate(glm::radians(m_pitch), glm::vec3(1, 0, 0));
    math::Matrix4x4f trans = m_root.GetLocalMatrix() * rotation;
    m_camera.Update(trans);
}

}  // namespace cave
