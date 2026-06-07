// =============================================================================
// File: cave/core/time/FrameTime.h
// =============================================================================
#pragma once

namespace cave {

struct FrameTime {
    float dt;
    uint64_t frame_index;
};

}  // namespace cave