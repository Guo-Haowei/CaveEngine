#pragma once
#include "engine/private/renderer/graphics_defines.h"
#include "engine/private/renderer/pipeline_state.h"

namespace cave {

class IRenderDevice;

class PipelineStateManager {
public:
    PipelineStateManager(IRenderDevice* p_graphics_manager)
        : m_render_device(p_graphics_manager) {}

    virtual ~PipelineStateManager() = default;

    auto Initialize() -> Result<void>;
    void Finalize();

    PipelineState* Find(PipelineStateName p_name);

    static const BlendDesc& GetBlendDescDefault();
    static const BlendDesc& GetBlendDescDisable();

protected:
    virtual auto CreateGraphicsPipeline(const PipelineStateDesc& p_desc) -> Result<std::shared_ptr<PipelineState>> = 0;
    virtual auto CreateComputePipeline(const PipelineStateDesc& p_desc) -> Result<std::shared_ptr<PipelineState>> = 0;

    IRenderDevice* m_render_device = nullptr;

private:
    auto Create(PipelineStateName p_name, const PipelineStateDesc& p_desc) -> Result<void>;

    std::array<std::shared_ptr<PipelineState>, PSO_NAME_MAX> m_cache;
};

}  // namespace cave