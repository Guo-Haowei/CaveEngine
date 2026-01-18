#pragma once
#include "engine/math/angle.h"
#include "engine/math/geomath.h"

namespace cave {

class CameraComponent;
class Scene;

struct ViewInfo {
    Matrix4x4f viewMatrix;
    Matrix4x4f projectionMatrixRendering;
    Matrix4x4f projectionMatrixFrustum;
    Vector3f position;
    Vector3f up;
    Vector3f front;
    Vector3f right;
    float sceenWidth;
    float sceenHeight;
    float aspectRatio;
    Degree fovy;

    static void FromCamera(const CameraComponent& p_camera,
                           ViewInfo& p_out_view_info,
                           bool p_is_opengl);
};

struct SceneView {
    ViewInfo view_info;
    Scene* scene{ nullptr };
};

class ISceneViewProvider {
public:
    virtual ~ISceneViewProvider() = default;

    virtual void Update(float p_timestep, bool p_focused) = 0;

    virtual void BuildViews(std::vector<SceneView>& p_out_views) = 0;

    virtual const char* GetDebugName() const = 0;
};

}  // namespace cave