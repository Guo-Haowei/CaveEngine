#pragma once
#include "engine/private/runtime/framework/PipelineStateManager.h"

namespace cave::render {

using rhi::Backend;

struct OpenGlPipelineState : public PipelineState {
    using PipelineState::PipelineState;

    uint32_t programId;

    ~OpenGlPipelineState();
};

class OpenGlPipelineStateManager : public PipelineStateManager {
public:
    explicit OpenGlPipelineStateManager() noexcept
        : PipelineStateManager(Backend::OpenGL) {}

    auto graphicsPipeline(const PipelineStateDesc& desc) -> Result<std::shared_ptr<PipelineState>> final;
    auto computePipeline(const PipelineStateDesc& desc) -> Result<std::shared_ptr<PipelineState>> final;

private:
    auto CreatePipelineImpl(const PipelineStateDesc& desc) -> Result<std::shared_ptr<PipelineState>>;
};

}  // namespace cave::render