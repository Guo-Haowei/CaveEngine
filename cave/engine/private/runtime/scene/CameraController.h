#pragma once
#include "cave/core/ids/Entity.h"
// @TODO: move to public
#include "engine/private/runtime/scene/TransformComponent.h"

namespace cave {

class CameraComponent;
class Scene;

struct CameraInputState {
    math::Vector3f move{ 0, 0, 0 };
    float zoom_delta{ 0 };
    math::Vector2f rotation{ 0, 0 };
};

class ICameraController {
public:
    virtual ~ICameraController() = default;
    virtual void Update(const CameraInputState& p_state) = 0;
};

class CameraController2DEditor : public ICameraController {
public:
    CameraController2DEditor(CameraComponent& p_camera,
                             TransformComponent& p_transform);

    void Update(const CameraInputState& p_state) final;

private:
    CameraComponent& m_camera;
    TransformComponent& m_root;
};

class CameraControllerFPS : public ICameraController {
public:
    CameraControllerFPS(CameraComponent& p_camera,
                        TransformComponent& p_transform);

    void Update(const CameraInputState& p_state) final;

private:
    CameraComponent& m_camera;
    TransformComponent& m_root;
    float m_pitch{ 0.0f };

    float m_move_speed{ 10.0f };
    float m_rotate_speed{ 10.0f };
    float m_scroll_speed{ 2.0f };
};

}  // namespace cave
