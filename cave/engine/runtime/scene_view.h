#pragma once
#include <bitset>

#include "engine/math/angle.h"
#include "engine/math/geomath.h"
#include "engine/input/key_code.h"

namespace cave {

class CameraComponent;
class Scene;

struct ViewInfo {
    Matrix4x4f view;
    Matrix4x4f projection_rendering;
    Matrix4x4f projection_frustum;
    Vector3f position;
    Vector3f up;
    Vector3f front;
    Vector3f right;
    float sceen_width;
    float sceen_height;
    float aspect_ratio;
    float fovy;

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

    virtual void BuildViews(std::vector<SceneView>& p_out_views, bool p_is_opengl) = 0;
};

}  // namespace cave