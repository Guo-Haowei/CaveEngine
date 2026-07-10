#pragma once
#include "cave/core/Color.h"

#include "engine/private/core/math/geomath.h"

// clang-format off
namespace cave { struct ImageAsset; }
// clang-format on
struct ImVec2;

namespace cave::ui {

enum class IconType {
    Info,
    Check,
    Exclamation,
};

void ColorIcon(Color p_color, IconType p_icon);

static inline void ColorIcon(ColorCode p_color, IconType p_icon) {
    ColorIcon(Color::Hex(p_color), p_icon);
}

void TraceIcon();

void InfoIcon();

void OkIcon();

void WarningIcon();

void ErrorIcon();

void CenteredImage(uint64_t handle,
                   int desired_size,
                   int img_width,
                   int img_height,
                   bool flip);

void CenteredImage(const ImageAsset* image,
                   const math::Vec2f& background_region,
                   uint64_t background);

using AssetCardImageFn = std::function<void(uint64_t, const ImVec2&, const ImVec2&)>;

auto AssetCard(uint64_t texture_id,
               const char* name,
               const math::Vec2f& image_size,
               AssetCardImageFn&& image_func = nullptr)
    -> std::tuple<bool, bool>;

}  // namespace cave::ui
