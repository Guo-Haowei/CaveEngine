#pragma once
#include "engine/math/geomath.h"
#include "engine/scene/scriptable_entity.h"

namespace cave {

class CameraComponent;

struct CameraInputState {
    Vector3f move{ 0, 0, 0 };
    float zoomDelta{ 0 };
    Vector2f rotation{ 0, 0 };
};

class ICameraController {
public:
    virtual ~ICameraController() = default;
    virtual void Update(CameraComponent& p_camera, const CameraInputState& p_state) = 0;
};

class CameraController2DEditor : public ICameraController {
public:
    void Update(CameraComponent& p_camera, const CameraInputState& p_state) override;
};

class CameraControllerFPS : public ICameraController {
public:
    void Update(CameraComponent& p_camera, const CameraInputState& p_state) override;

    float m_moveSpeed{ 10.0f };
    float m_rotateSpeed{ 10.0f };
    float m_scrollSpeed{ 2.0f };
};

}  // namespace cave
