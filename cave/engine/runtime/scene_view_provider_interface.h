#pragma once
#include "engine/math/angle.h"
#include "engine/math/geomath.h"

namespace cave {

class CameraComponent;

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

class ISceneViewProvider {
public:
    virtual ~ISceneViewProvider() = default;

    // virtual void Tick(float dt, const FViewportInput& input, bool bFocused) = 0;

    // 1..N views
    // virtual void BuildViews(const FIntRect& viewRect,
    //                        FRenderTarget* rt,
    //                        std::vector<FRenderView>& outViews) = 0;

    virtual const char* GetDebugName() const = 0;
};

}  // namespace cave