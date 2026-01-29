#pragma once
#include "cave/core/ids/Entity.h"
#include "engine/private/math/geomath.h"

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
    CameraController2DEditor(Scene* p_scene,
                             ecs::Entity p_cam)
        : m_scene(p_scene)
        , m_cam(p_cam) {}

    void Update(const CameraInputState& p_state) final;

private:
    Scene* m_scene;
    ecs::Entity m_cam;
};

class CameraControllerFPS : public ICameraController {
public:
    CameraControllerFPS(Scene* p_scene,
                        ecs::Entity p_cam_root,
                        ecs::Entity p_cam_y,
                        ecs::Entity p_cam)
        : m_scene(p_scene)
        , m_cam_root(p_cam_root)
        , m_cam_y(p_cam_y)
        , m_cam(p_cam) {}

    void Update(const CameraInputState& p_state) final;

private:
    Scene* m_scene;
    ecs::Entity m_cam_root;
    ecs::Entity m_cam_y;
    ecs::Entity m_cam;

    float m_move_speed{ 10.0f };
    float m_rotate_speed{ 10.0f };
    float m_scroll_speed{ 2.0f };
};

}  // namespace cave
