#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

namespace cave {

class CameraComponent;
class Scene;
struct InputFrame;

class ICameraController {
public:
    virtual ~ICameraController() = default;
    virtual void update(const InputFrame& input) = 0;

    virtual void setMoveSpeed(float) {}
};

class CameraController2DEditor : public ICameraController {
public:
    static constexpr float kDefaultPanSpeed = 1.0f;

    CameraController2DEditor(CameraComponent& camera,
                             TransformComponent& transform);

    void update(const InputFrame& input) final;

    void setMoveSpeed(float value) override { m_move_speed = value; }

private:
    CameraComponent& m_camera;
    TransformComponent& m_root;

    float m_move_speed = kDefaultPanSpeed;
};

class CameraControllerFPS : public ICameraController {
public:
    CameraControllerFPS(CameraComponent& camera,
                        TransformComponent& transform);

    void update(const InputFrame& input) final;

private:
    CameraComponent& m_camera;
    TransformComponent& m_root;
    float m_pitch{ 0.0f };

    float m_move_speed{ 10.0f };
    float m_rotate_speed{ 10.0f };
    float m_scroll_speed{ 2.0f };
};

}  // namespace cave
