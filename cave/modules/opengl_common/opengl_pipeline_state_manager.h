#pragma once
#include "engine/runtime/pipeline_state_manager.h"

namespace cave {

struct OpenGlPipelineState : public PipelineState {
    using PipelineState::PipelineState;

    uint32_t programId;

    ~OpenGlPipelineState();
};

class OpenGlPipelineStateManager : public PipelineStateManager {
public:
    OpenGlPipelineStateManager(IGraphicsManager* p_graphics_manager)
        : PipelineStateManager(p_graphics_manager) {}

    auto CreateGraphicsPipeline(const PipelineStateDesc& p_desc) -> Result<std::shared_ptr<PipelineState>> final;
    auto CreateComputePipeline(const PipelineStateDesc& p_desc) -> Result<std::shared_ptr<PipelineState>> final;

private:
    auto CreatePipelineImpl(const PipelineStateDesc& p_desc) -> Result<std::shared_ptr<PipelineState>>;
};

}  // namespace cave