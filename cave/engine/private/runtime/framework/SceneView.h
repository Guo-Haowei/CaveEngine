#pragma once
#include <bitset>
#include "cave/core/ids/DebugId.h"
#include "cave/core/ids/SceneId.h"

#include "cave/core/math/Angle.h"
#include "engine/private/math/geomath.h"

namespace cave {

class CameraComponent;
class Scene;
class ISceneRegistry;

struct ViewInfo {
    math::Matrix4x4f view;
    math::Matrix4x4f projection_rendering;
    math::Matrix4x4f projection_frustum;
    math::Vector3f position;
    math::Vector3f up;
    math::Vector3f front;
    math::Vector3f right;
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
    ISceneRegistry* scene_manager{ nullptr };

    Scene* ResolveScene();
};

class ISceneViewProvider {
public:
    virtual ~ISceneViewProvider() = default;

    virtual void BuildViews(std::vector<SceneView>& p_out_views, bool p_is_opengl) = 0;

    virtual DebugId GetDebugId() = 0;
};

}  // namespace cave