#pragma once
#include "engine/private/render/render_graph/RGTextureId.h"

#include "engine/private/renderer/gpu_resource.h"

namespace cave::render {

struct RenderOptions;
class IRenderDevice;
class RenderGraph;
class TransientPool;

class EnvironmentFeature {
public:
    explicit EnvironmentFeature(TransientPool& p_pool, IRenderDevice& p_device) noexcept
        : m_pool(p_pool)
        , m_device(p_device) {}

    struct Outputs {
        RGTextureId skybox{};
        RGTextureId ibl_diffuse{};
        RGTextureId ibl_prefiltered{};
    };

    [[nodiscard]] Outputs Build(RenderGraph& p_graph, const RenderOptions& p_plan);

private:
    TransientPool& m_pool;
    IRenderDevice& m_device;
    GpuTextureId m_env_texture{};
    GpuTextureId m_env_cube{};
    GpuTextureId m_diffuse{};
    GpuTextureId m_specular{};
};

}  // namespace cave::render
