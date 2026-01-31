#pragma once
#include "engine/private/renderer/frame_data.h"

namespace cave::render {

struct FramePlan {
    bool enable_ssao{ false };
    bool enable_bloom{ false };

    std::vector<FrameData> frame_data;
};

}  // namespace cave::render
