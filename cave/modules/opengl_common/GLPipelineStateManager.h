#pragma once
#include "engine/private/runtime/framework/PipelineStateManager.h"

namespace cave::render {

using rhi::Backend;

struct OpenGlPipelineState : public PipelineState {
    using PipelineState::PipelineState;

    uint32_t programId;

    ~OpenGlPipelineState();
};

class GLPipelineStateManager : public PipelineStateManager {
public:
    explicit GLPipelineStateManager() noexcept
        : PipelineStateManager(Backend::OpenGL) {}

    auto graphicsPipeline(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>> final;
    auto computePipeline(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>> final;

private:
    auto CreatePipelineImpl(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>>;
};

}  // namespace cave::render