#pragma once
#include "cave/rhi/Backend.h"

#include "engine/private/renderer/graphics_defines.h"
#include "engine/private/render/rhi/PipelineState.h"

namespace cave::render {

using rhi::Backend;

class IRenderDevice;
struct RenderCapabilities; 

class PipelineStateManager {
public:
    explicit PipelineStateManager(Backend backend) noexcept
        : m_backend(backend) {}

    virtual ~PipelineStateManager() = default;

    auto initialize(const RenderCapabilities& capabilities) -> Result<void>;
    void finalize();

    PipelineState* findPSO(PipelineStateName name);

    static const BlendDesc& defaultBlendDesc();
    static const BlendDesc& blendDescDisabled();

protected:
    virtual auto graphicsPipeline(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>> = 0;
    virtual auto computePipeline(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>> = 0;

    const Backend m_backend;

private:
    auto create(PipelineStateName name, const PipelineStateDesc& desc) -> Result<void>;

    std::array<Owner<PipelineState>, PSO_NAME_MAX> m_pso_cache;
};

}  // namespace cave::render