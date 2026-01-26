#pragma once
#include <bitset>
#include "cave/runtime/scene/SceneId.h"

#include "engine/private/math/angle.h"
#include "engine/private/math/geomath.h"

namespace cave {

class CameraComponent;
class Scene;
class SceneManager;

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
    SceneId scene_id;
    SceneManager* scene_manager{ nullptr };

    Scene* ResolveScene();
};

class ISceneViewProvider {
public:
    virtual ~ISceneViewProvider() = default;

    virtual void BuildViews(std::vector<SceneView>& p_out_views, bool p_is_opengl) = 0;
};

}  // namespace cave