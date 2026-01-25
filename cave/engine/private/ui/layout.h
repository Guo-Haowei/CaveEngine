#pragma once
#include "engine/math/geomath.h"

namespace cave {

struct AssetChildPanel {
    const char* name;
    float width;
    std::function<void()> func;
};

}  // namespace cave

namespace cave::ui {

struct DockSpaceContext {
    const char* str_id{ nullptr };
    std::function<void()> menu_bar_func;
    std::function<void()> side_bar_func;
};

void DockSpace(const DockSpaceContext& p_context);

/// asset inspector
void DrawContents(float p_full_width, const std::vector<AssetChildPanel>& p_descs);

}  // namespace cave::ui
