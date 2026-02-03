#pragma once
#include "engine/private/render/render_graph/RGTextureId.h"

#include "engine/private/renderer/gpu_resource.h"

namespace cave::render {

struct RenderOptions;
class RenderGraph;
class TransientPool;

class EnvironmentFeature {
public:
    EnvironmentFeature(TransientPool& p_pool)
        : m_pool(p_pool) {}

    struct Outputs {
        RGTextureId skybox{};
        RGTextureId ibl_diffuse{};
        RGTextureId ibl_prefiltered{};
    };

    [[nodiscard]] Outputs Build(RenderGraph& p_graph, const RenderOptions& p_plan);

private:
    TransientPool& m_pool;
    GpuTextureId m_env_texture{};
    bool m_generated{ false };
};

}  // namespace cave::render
