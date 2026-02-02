#pragma once
#include "engine/private/renderer/frame_data.h"

namespace cave::render {

struct FramePlan {
    bool enable_ssao{ true };
    bool enable_bloom{ true };

    std::vector<FrameData> frame_data;
};

}  // namespace cave::render
