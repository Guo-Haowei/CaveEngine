#pragma once
#include "engine/ecs/entity.h"
#include "engine/math/geomath.h"

namespace cave {

class CameraComponent;
class Scene;

struct CameraInputState {
    Vector3f move{ 0, 0, 0 };
    float zoom_delta{ 0 };
    Vector2f rotation{ 0, 0 };
};

class ICameraController {
public:
    virtual ~ICameraController() = default;
    virtual void Update(const CameraInputState& p_state) = 0;
};

class CameraController2DEditor : public ICameraController {
public:
    void Update(const CameraInputState& p_state) override;
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
        , m_cam(p_cam)
    {}

    void Update(const CameraInputState& p_state) override;

    float m_moveSpeed{ 10.0f };
    float m_rotateSpeed{ 10.0f };
    float m_scrollSpeed{ 2.0f };

private:
    Scene* m_scene;
    ecs::Entity m_cam_root;
    ecs::Entity m_cam_y;
    ecs::Entity m_cam;
};

}  // namespace cave
