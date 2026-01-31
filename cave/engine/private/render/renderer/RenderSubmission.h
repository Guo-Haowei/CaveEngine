#pragma once

#include "engine/private/render/render_graph/RenderGraph.h"
// @TODO: refactor frame_data
#include "engine/private/renderer/frame_data.h"

namespace cave::render {

struct RenderSubmission {
    uint64_t frame_index;
    std::vector<FrameData> frame_data;
    RenderGraph graph;
};

}  // namespace cave::render
