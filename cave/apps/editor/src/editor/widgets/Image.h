#pragma once
#include "engine/private/core/math/geomath.h"

// clang-format off
namespace cave { struct ImageAsset; }
// clang-format on
struct ImVec2;

namespace cave::ui {

void OkIcon();

void WarningIcon();

void ErrorIcon();

void CenteredImage(uint64_t p_handle,
                   int p_desired_size,
                   int p_img_width,
                   int p_img_height,
                   bool p_flip);

void CenteredImage(const ImageAsset* p_image,
                   const math::Vector2f& p_background_region,
                   uint64_t p_background);

auto AssetCard(uint64_t p_texture_id,
               const char* p_name,
               const math::Vector2f& p_image_size) -> std::tuple<bool, bool>;

}  // namespace cave::ui
