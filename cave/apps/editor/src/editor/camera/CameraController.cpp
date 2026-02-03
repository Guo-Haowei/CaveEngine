#include "CameraController.h"

#include "cave/core/math/Angle.h"
#include "cave/runtime/scene/CameraComponent.h"

#include "engine/private/runtime/framework/InputSystem.h"

namespace cave {

using math::Matrix4x4f;
using math::Vector3f;

CameraController2DEditor::CameraController2DEditor(CameraComponent& p_camera,
                                                   TransformComponent& p_tranform)
    : m_camera(p_camera)
    , m_root(p_tranform) {
}

void CameraController2DEditor::Update(const InputFrame& p_input) {
    unused(p_input);
#if 0
    bool need_update = false;
    if (dx || dy || dz) {
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
#endif
}

CameraControllerFPS::CameraControllerFPS(CameraComponent& p_camera,
                                         TransformComponent& p_tranform)
    : m_camera(p_camera)
    , m_root(p_tranform) {
}

void CameraControllerFPS::Update(const InputFrame& p_input) {
    constexpr Key kDragKey = Key::MMB;
    const InputDeviceId id{ 0 };
    auto& ks = p_input.keystate;
    const bool drag_button = ks.Down(id, kDragKey);
    const int _dx = ks.Down(id, Key::D) - ks.Down(id, Key::A);
    const int _dy = ks.Down(id, Key::E) - ks.Down(id, Key::Q);
    const int _dz = ks.Down(id, Key::W) - ks.Down(id, Key::S);

    math::Vector2f rotation = math::Vector2f::Zero;

    for (const InputEvent& e : p_input.events) {
        if (e.consumed) continue;
        switch (e.type) {
            case InputEventType::MouseMove: {
                if (drag_button) {
                    e.consumed = true;
                    rotation.x = e.dx;
                    rotation.y = e.dy;
                }
            } break;
            default:
                break;
        }
    }

    const float dt = p_input.dt;
    const bool moved = _dx || _dy || _dz;
    if (moved) {
        float dx = dt * _dx;
        float dy = dt * _dy;
        float dz = dt * _dz;

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

        math::Vector2f movement = rotation;
        movement = m_rotate_speed * movement;
        if (glm::abs(movement.x) > glm::abs(movement.y)) {
            rotate_y = movement.x;
        } else {
            rotate_x = movement.y;
        }

        if (rotate_y) {
            m_root.RotateY(math::Degree(-rotate_y * dt));
        }

        if (rotate_x) {
            m_pitch += rotate_x * dt;
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
    math::Matrix4x4f R = glm::rotate(glm::radians(m_pitch), glm::vec3(1, 0, 0));
    math::Matrix4x4f trans = m_root.GetLocalMatrix() * R;
    m_camera.Update(trans);
}

#if 0
CameraInputState SceneViewTab::CreateCameraInputState2D(const std::vector<InputEvent>& p_events, const KeyState& p_ks) {
    CameraInputState state{};

    const InputDeviceId id{ 0 };
    float dx = 0.0f;
    float dy = 0.0f;
    const bool mmb = p_ks.Down(id, Key::MMB);

    for (const InputEvent& e : p_events) {
        if (e.consumed) {
            continue;
        }

        switch (e.type) {
            case InputEventType::MouseWheel: {
                e.consumed = true;
                state.zoom_delta = -e.dy;
            } break;
            case InputEventType::MouseMove: {
                e.consumed = true;
                dx = -e.dx;
                dy = e.dy;
            } break;
            default:
                break;
        }
    }

    if (mmb) {
        state.move = math::Vector3f(dx, dy, 0.0f);
    }

    return state;
}

#endif

}  // namespace cave
