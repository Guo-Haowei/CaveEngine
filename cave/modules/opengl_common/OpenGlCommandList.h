#pragma once
#include "cave/render/rhi/ICommandList.h"

#include "OpenGlDefines.h"

namespace cave::render {

class OpenGlFramebufferCache;
class OpenGlPipelineStateManager;

class OpenGlCommandList final : public ICommandList {
public:
    explicit OpenGlCommandList(OpenGlFramebufferCache& fbo_cache,
                               OpenGlPipelineStateManager& pso_manager)
        : m_fbo_cache(fbo_cache)
        , m_pipeline_state_manager(pso_manager) {}

    void draw(uint32_t count,
              uint32_t offset) override;
    void drawInstanced(uint32_t instance_count,
                       uint32_t count,
                       uint32_t offset) override;
    void drawIndexed(uint32_t count,
                     uint32_t offset) override;
    void drawIndexedInstanced(uint32_t instance_count,
                              uint32_t count,
                              uint32_t offset) override;

    void beginRendering(const RenderingInfo& info) override;
    void endRendering() override;

    void pushDebugGroup(std::string_view name) override;
    void popDebugGroup() override;

    void setViewport(const Viewport& viewport) override;

    void setPipelineState(PipelineStateName name) override;

    void setStencilRef(uint32_t ref) override;

    void bindConstantBufferRange(const GpuResource* buffer,
                                 uint32_t size,
                                 uint32_t offset) override;

    void dispatch(uint32_t num_groups_x,
                  uint32_t num_groups_y,
                  uint32_t num_groups_z) override;

    // @TODO: remove these
    void setMesh(const GpuMesh* mesh) override;

    void bindTexture(Dimension dimension, uint64_t handle, int slot) override;
    void unbindTexture(Dimension dimension, int slot) override;

    void bindUnorderedAccessView(uint32_t slot, GpuResource* texture) override;
    void unbindUnorderedAccessView(uint32_t slot) override;

private:
    void setBlendState(const BlendDesc& desc, const float* factor, uint32_t mask);

    // @TODO: refactor
    OpenGlFramebufferCache& m_fbo_cache;
    OpenGlPipelineStateManager& m_pipeline_state_manager;

    struct {
        CullMode cullMode;
        bool frontCounterClockwise;
        ComparisonFunc depthFunc;
        bool enableDepthTest;
        bool enableStencilTest;
        ComparisonFunc stencilFunc;
        gl::TOPOLOGY topology;
    } m_stateCache;

    uint32_t m_dummy_vao{ 0 };  // for drawing with gl_VertexID
};

}  // namespace cave::render
