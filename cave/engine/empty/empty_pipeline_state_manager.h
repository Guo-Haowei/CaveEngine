#pragma once
#include "engine/runtime/pipeline_state_manager.h"

namespace cave {

class EmptyPipelineStateManager : public PipelineStateManager {
public:
    EmptyPipelineStateManager(IGraphicsManager* p_graphics_manager)
        : PipelineStateManager(p_graphics_manager) {}

protected:
    auto CreateGraphicsPipeline(const PipelineStateDesc& p_desc) -> Result<std::shared_ptr<PipelineState>> override {
        unused(p_desc);
        return CAVE_ERROR(ErrorCode::FAILURE);
    }

    auto CreateComputePipeline(const PipelineStateDesc& p_desc) -> Result<std::shared_ptr<PipelineState>> override {
        unused(p_desc);
        return CAVE_ERROR(ErrorCode::FAILURE);
    }
};

}  // namespace cave
