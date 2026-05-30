#pragma once
#include "ResolvedView.h"
// @TODO: fix
#include "engine/private/render/renderer/FrameData.h"

namespace cave::render {

struct FramePlan {
    std::vector<ResolvedView> views;
    std::vector<FrameData> frame_data;
};

}  // namespace cave::render
