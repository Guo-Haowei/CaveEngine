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

enum class PathTracerMode {
    None,
    Interactive,
};

class PathTracerFeature {
public:
    struct Inputs {
        RGTextureId skybox{};
        GpuTextureId out{};
    };


    void build(RenderGraph& graph,
               const Inputs& inputs,
               const RenderOptions& plan);

    void update(Scene& scene);

    bool isActive() const;
    void setMode(PathTracerMode mode);
};

}  // namespace cave::render
