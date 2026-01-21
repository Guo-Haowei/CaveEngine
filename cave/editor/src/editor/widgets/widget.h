#pragma once
#include "engine/math/geomath.h"

namespace cave {

/// asset inspector
struct AssetChildPanel {
    const char* name;
    float width;
    std::function<void()> func;
};

void DrawContents(float p_full_width, const std::vector<AssetChildPanel>& p_descs);

}  // namespace cave
