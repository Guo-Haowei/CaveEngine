#pragma once
#include "engine/private/runtime/framework/PipelineStateManager.h"

namespace cave::render {

using rhi::Backend;

class NullPipelineStateManager : public PipelineStateManager {
public:
    explicit NullPipelineStateManager() noexcept
        : PipelineStateManager(Backend::Null) {}

protected:
    auto graphicsPipeline(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>> override {
        unused(desc);
        return CAVE_ERROR(ErrorCode::FAILURE);
    }

    auto computePipeline(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>> override {
        unused(desc);
        return CAVE_ERROR(ErrorCode::FAILURE);
    }
};

}  // namespace cave::render
