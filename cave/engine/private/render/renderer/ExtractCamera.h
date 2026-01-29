#pragma once
#include "cave/render/CameraParams.h"

// clang-format off
namespace cave { class CameraComponent; }
// clang-format on

namespace cave::render {

void ExtractCamera(const CameraComponent& p_camera,
                   bool p_is_opengl,
                   CameraParams& p_out_cam);

}  // namespace cave::render
