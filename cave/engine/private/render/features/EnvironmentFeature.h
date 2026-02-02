#pragma once
#include "engine/private/render/render_graph/RGTextureId.h"

#include "engine/private/renderer/gpu_resource.h"

namespace cave::render {

struct FramePlan;
class RenderGraph;

class EnvironmentFeature {
public:
    struct Outputs {
        RGTextureId skybox{};
        RGTextureId ibl_diffuse{};
        RGTextureId ibl_prefiltered{};
    };

    [[nodiscard]] Outputs Build(RenderGraph& p_graph, const FramePlan& p_plan);

private:
    GpuTextureId m_env_texture{};
};

}  // namespace cave::render
