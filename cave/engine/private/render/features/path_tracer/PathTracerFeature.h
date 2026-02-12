#pragma once
#include "engine/private/render/render_graph/RGTextureId.h"

// @TODO: refactor this
#include "engine/private/renderer/gpu_resource.h"

namespace cave {
class Scene;
}  // namespace cave

namespace cave::render {

struct RenderOptions;
class IRenderDevice;
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

enum class PathTracerMode {
    NONE,
    INTERACTIVE,
    TILED,
};

// @TODO: refactor
void RequestPathTracerUpdate(Scene& p_scene);
void SetPathTracerMode(PathTracerMode p_mode);
bool IsPathTracerActive();
void BindPathTracerData(IRenderDevice& p_device);
void UnbindPathTracerData(IRenderDevice& p_device);

}  // namespace cave::render
