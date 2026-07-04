#pragma once
#include "cave/rhi/Backend.h"

#include "engine/private/renderer/graphics_defines.h"
#include "engine/private/render/rhi/PipelineState.h"

namespace cave::render {

using rhi::Backend;

class IRenderDevice;

class PipelineStateManager {
public:
    explicit PipelineStateManager(Backend backend) noexcept
        : backend_(backend) {}

    virtual ~PipelineStateManager() = default;

    auto initialize() -> Result<void>;
    void finalize();

    PipelineState* findPSO(PipelineStateName name);

    static const BlendDesc& defaultBlendDesc();
    static const BlendDesc& blendDescDisabled();

protected:
    virtual auto graphicsPipeline(const PipelineStateDesc& desc) -> Result<std::shared_ptr<PipelineState>> = 0;
    virtual auto computePipeline(const PipelineStateDesc& desc) -> Result<std::shared_ptr<PipelineState>> = 0;

    const Backend backend_;

private:
    auto create(PipelineStateName name, const PipelineStateDesc& desc) -> Result<void>;

    std::array<std::shared_ptr<PipelineState>, PSO_NAME_MAX> pso_cache_;
};

}  // namespace cave::render