#pragma once
#include "engine/private/runtime/framework/PipelineStateManager.h"

namespace cave::render {

class EmptyPipelineStateManager : public PipelineStateManager {
public:
    EmptyPipelineStateManager(IRenderDevice* p_graphics_manager)
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

}  // namespace cave::render
