#pragma once
#include "engine/private/render/render_graph/RGTextureId.h"

// @TODO: refactor this
#include "engine/private/renderer/gpu_resource.h"

namespace cave::render {

struct RenderOptions;
class RenderGraph;

class PathTracerFeature {
public:
    struct Inputs {
        GpuTextureId out{};
    };

    struct Outputs {};

    Outputs Build(RenderGraph& p_graph,
                  const RenderOptions& p_plan,
                  const Inputs& p_in);
};

}  // namespace cave::render
