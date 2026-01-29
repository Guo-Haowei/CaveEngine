// =============================================================================
// File: public/cave/render/CameraParams.h
// =============================================================================
#pragma once
#include "cave/core/math/Matrix.h"

namespace cave::render {

struct CameraParams {
    math::Matrix4x4f view;
    math::Matrix4x4f proj_rendering;
    math::Matrix4x4f proj_culling;

    math::Vector3f position;
    math::Vector3f up;
    math::Vector3f front;
    math::Vector3f right;

    float vp_w;
    float vp_h;
    float aspect;
    float fovy;
};

}  // namespace cave::render