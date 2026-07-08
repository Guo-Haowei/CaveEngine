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
};

class CameraController2DEditor : public ICameraController {
public:
    CameraController2DEditor(CameraComponent& camera,
                             TransformComponent& transform);

    void update(const InputFrame& input) final;

private:
    CameraComponent& camera_;
    TransformComponent& root_;
};

class CameraControllerFPS : public ICameraController {
public:
    CameraControllerFPS(CameraComponent& camera,
                        TransformComponent& transform);

    void update(const InputFrame& input) final;

private:
    CameraComponent& camera_;
    TransformComponent& root_;
    float pitch_{ 0.0f };

    float move_speed_{ 10.0f };
    float rotate_speed_{ 10.0f };
    float scroll_speed_{ 2.0f };
};

}  // namespace cave
