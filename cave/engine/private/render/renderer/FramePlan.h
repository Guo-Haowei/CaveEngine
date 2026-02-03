#pragma once
#include "ResolvedView.h"
// @TODO: fix
#include "engine/private/renderer/frame_data.h"

namespace cave::render {

struct FramePlan {
    std::vector<ResolvedView> views;
    std::vector<FrameData> frame_data;
};

}  // namespace cave::render
