#pragma once
#include "engine/math/geomath.h"

namespace cave {

class CameraComponent;
class TransformComponent;

struct CameraInputState {
    Vector3f move{ 0, 0, 0 };
    float zoom_delta{ 0 };
    Vector2f rotation{ 0, 0 };
};

class ICameraController {
public:
    virtual ~ICameraController() = default;
    virtual void Update(const CameraInputState& p_state,
                        CameraComponent& p_camera,
                        TransformComponent& p_transform) = 0;
};

class CameraController2DEditor : public ICameraController {
public:
    void Update(const CameraInputState& p_state,
                CameraComponent& p_camera,
                TransformComponent& p_transform) override;
};

class CameraControllerFPS : public ICameraController {
public:
    void Update(const CameraInputState& p_state,
                CameraComponent& p_camera,
                TransformComponent& p_transform) override;

    float m_moveSpeed{ 10.0f };
    float m_rotateSpeed{ 10.0f };
    float m_scrollSpeed{ 2.0f };
};

}  // namespace cave
