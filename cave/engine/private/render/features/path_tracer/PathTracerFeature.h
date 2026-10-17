#pragma once
#include "cave/render/rhi/GpuResources.h"

// @TODO: refactor this
#include "engine/private/render/render_graph/RGTextureId.h"

namespace cave {
class Scene;
}  // namespace cave

namespace cave::render {

struct RenderOptions;
class ICommandList;
class RenderGraph;

class PathTracerFeature {
public:
    struct Inputs {
        RGTextureId skybox{};

        GpuTextureId out{};
    };

    struct Outputs {};

    Outputs build(RenderGraph& graph,
                  const Inputs& inputs,
                  const RenderOptions& plan);
};

enum class PathTracerMode {
    NONE,
    INTERACTIVE,
    TILED,
};

// @TODO: refactor
void RequestPathTracerUpdate(Scene& p_scene);
void SetPathTracerMode(PathTracerMode p_mode);
bool IsPathTracerActive();

}  // namespace cave::render
