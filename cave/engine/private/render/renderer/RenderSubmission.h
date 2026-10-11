#pragma once

#include "engine/private/render/render_graph/CompiledGraph.h"
// @TODO: refactor frame_data
#include "engine/private/render/renderer/FrameData.h"

namespace cave::render {

struct RenderSubmission {
    uint64_t frame_index;
    Vector<FrameData> frame_data;
    Vector<Ref<CompiledGraph>> render_graph;
};

}  // namespace cave::render
