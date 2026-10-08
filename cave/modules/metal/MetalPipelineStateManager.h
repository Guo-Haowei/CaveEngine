#pragma once

#include "engine/private/runtime/framework/PipelineStateManager.h"

namespace cave::render {

struct MetalPipelineState final : PipelineState {
    using PipelineState::PipelineState;
    void* state{};
    void* depth_state{};
    ~MetalPipelineState() override;
};

class MetalPipelineStateManager final : public PipelineStateManager {
public:
    MetalPipelineStateManager() noexcept : PipelineStateManager(rhi::Backend::Metal) {}
    auto graphicsPipeline(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>> final;
    auto computePipeline(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>> final;
};

} // namespace cave::render
