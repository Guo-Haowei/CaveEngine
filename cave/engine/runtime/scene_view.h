#pragma once
#include <bitset>

#include "engine/math/angle.h"
#include "engine/math/geomath.h"
#include "engine/input/input_code.h"

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
    Degree fovy;

    static void FromCamera(const CameraComponent& p_camera,
                           ViewInfo& p_out_view_info,
                           bool p_is_opengl);
};

struct SceneView {
    ViewInfo view_info;
    Scene* scene{ nullptr };
};

struct ViewportInput {
    float wheel_delta{ 0 };
    Vector2f mouse_move{ 0, 0 };
    MouseButtonArray buttons{};
    KeyArray keys{};

    bool IsKeyDown(KeyCode p_key_code) const {
        return keys.test(std::to_underlying(p_key_code));
    }

    bool IsButtonDown(MouseButton p_button) const {
        return buttons.test(std::to_underlying(p_button));
    }
};

class ISceneViewProvider {
public:
    virtual ~ISceneViewProvider() = default;

    virtual void Update(float p_timestep,
                        const ViewportInput& p_input,
                        bool p_focused) = 0;

    virtual void BuildViews(std::vector<SceneView>& p_out_views,
                            bool p_is_opengl) = 0;

    virtual const char* GetDebugName() const = 0;
};

}  // namespace cave