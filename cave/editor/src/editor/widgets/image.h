#pragma once
#include "engine/math/geomath.h"

// clang-format off
namespace cave { struct ImageAsset; }
// clang-format on

namespace cave::ui {

void CenteredImage(const ImageAsset* p_image,
                   const Vector2f& p_background_region,
                   uint64_t p_background);

}  // namespace cave::ui
