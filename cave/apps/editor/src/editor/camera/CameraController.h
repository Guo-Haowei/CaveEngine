#pragma once
#include "cave/core/ids/Entity.h"
// @TODO: move to public
#include "engine/private/runtime/scene/TransformComponent.h"

namespace cave {

class CameraComponent;
class Scene;
struct InputFrame;

class ICameraController {
public:
    virtual ~ICameraController() = default;
    virtual void Update(const InputFrame& p_input) = 0;
};

class CameraController2DEditor : public ICameraController {
public:
    CameraController2DEditor(CameraComponent& p_camera,
                             TransformComponent& p_transform);

    void Update(const InputFrame& p_input) final;

private:
    CameraComponent& m_camera;
    TransformComponent& m_root;
};

class CameraControllerFPS : public ICameraController {
public:
    CameraControllerFPS(CameraComponent& p_camera,
                        TransformComponent& p_transform);

    void Update(const InputFrame& p_input) final;

private:
    CameraComponent& m_camera;
    TransformComponent& m_root;
    float m_pitch{ 0.0f };

    float m_move_speed{ 10.0f };
    float m_rotate_speed{ 10.0f };
    float m_scroll_speed{ 2.0f };
};

}  // namespace cave
