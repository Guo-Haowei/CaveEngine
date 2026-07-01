#include "CameraController.h"

#include "cave/core/math/Angle.h"
#include "cave/runtime/ecs/components/CameraComponent.h"

#include "engine/private/core/math/MatrixTransform.h"
#include "engine/private/runtime/input/InputService.h"

// @TODO: refactor
#include "cave/runtime/input/IInputConsumer.h"
#include "cave/runtime/input/KeyState.h"

namespace cave {

using namespace ::cave::math;

CameraController2DEditor::CameraController2DEditor(CameraComponent& camera,
                                                   TransformComponent& tranform)
    : camera_(camera)
    , root_(tranform) {
}

void CameraController2DEditor::update(const InputFrame& input) {
    constexpr Key drag_key = Key::MMB;
    const InputDeviceId id{ 0 };
    const bool drag_key_down = input.keystate.down(id, drag_key);
    float dx = 0.0f;
    float dy = 0.0f;
    float zoom = 0.0f;

    for (const InputEvent& e : input.events) {
        if (e.consumed) {
            continue;
        }

        switch (e.type) {
            case InputEventType::MouseWheel: {
                e.consumed = true;
                zoom = -e.dy;
            } break;
            case InputEventType::MouseMove: {
                e.consumed = true;
                dx = -e.dx;
                dy = e.dy;
            } break;
            case InputEventType::ButtonDown: {
                if (e.code == std::to_underlying(drag_key)) {
                    e.consumed = true;
                }
            } break;
            default:
                break;
        }
    }

    bool need_update = false;
    const float dt = input.dt;
    if (drag_key_down && (dx || dy)) {
        constexpr float speed = 1.0f;
        Vec2f delta(dx, dy);
        delta *= (speed * dt);
        root_.translate(Vec3f(delta, 0.0f));

        need_update = true;
    }

    if (zoom != 0.0f) {
        float ortho_height = camera_.orthoHeight() + 16.0f * (zoom * dt);
        ortho_height = math::clamp(ortho_height, 0.1f, 100.0f);
        camera_.setOrthoHeight(ortho_height);
        need_update = true;
    }

    if (need_update) {
        camera_.setDirty();
        root_.updateTransform();
        camera_.update(root_.worldMatrix());
    }
}

CameraControllerFPS::CameraControllerFPS(CameraComponent& camera,
                                         TransformComponent& tranform)
    : camera_(camera)
    , root_(tranform) {
}

void CameraControllerFPS::update(const InputFrame& input) {
    constexpr Key kDragKey = Key::MMB;
    const InputDeviceId id{ 0 };
    auto& ks = input.keystate;
    const bool drag_button = ks.down(id, kDragKey);
    const int _dx = ks.down(id, Key::D) - ks.down(id, Key::A);
    const int _dy = ks.down(id, Key::E) - ks.down(id, Key::Q);
    const int _dz = ks.down(id, Key::W) - ks.down(id, Key::S);

    Vec2f rotation = Vec2f::Zero;

    for (const InputEvent& e : input.events) {
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

    const float dt = input.dt;
    const bool moved = _dx || _dy || _dz;
    if (moved) {
        float dx = dt * _dx;
        float dy = dt * _dy;
        float dz = dt * _dz;

        if (dx || dz) {
            Vec3f delta = (move_speed_ * dz) * camera_.front() + (move_speed_ * dx) * camera_.right();
            root_.translate(delta);
        }
        if (dy) {
            root_.translate(Vec3f(0.0f, move_speed_ * dy, 0.0f));
        }
    }

    auto rotate_camera = [&]() {
        float rotate_x = 0.0f;
        float rotate_y = 0.0f;

        Vec2f movement = rotation;
        movement = rotate_speed_ * movement;
        if (glm::abs(movement.x) > glm::abs(movement.y)) {
            rotate_y = movement.x;
        } else {
            rotate_x = movement.y;
        }

        if (rotate_y) {
            root_.rotateY(Degree(-rotate_y * dt));
        }

        if (rotate_x) {
            pitch_ += rotate_x * dt;
            pitch_ = clamp(pitch_, -80.0f, 80.0f);
            camera_.setDirty();
        }

        return rotate_x != 0.0f || rotate_y != 0.0f;
    };

    if (moved || rotate_camera()) {
        camera_.setDirty();
    }
    if (root_.dirty()) {
        camera_.setDirty();
    }

    root_.updateTransform();
    Mat4f R = glm::rotate(glm::radians(pitch_), glm::vec3(1, 0, 0));
    Mat4f trans = root_.localMatrix() * R;
    camera_.update(trans);
}

}  // namespace cave
