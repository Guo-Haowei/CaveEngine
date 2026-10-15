#pragma once
#include "cave/render/rhi/ICommandList.h"

#include "engine/private/renderer/graphics_defines.h"

#include "OpenGlDefines.h"

namespace cave::render {

class OpenGlPipelineStateManager;

class OpenGlCommandList final : public ICommandList {
public:
    explicit OpenGlCommandList(OpenGlPipelineStateManager& pso_manager)
        : m_pipeline_state_manager(pso_manager) {}

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

    void bindConstantBufferRange(const GpuConstantBuffer* buffer,
                                 uint32_t size,
                                 uint32_t offset) override;

    void setPipelineState(PipelineStateName name) override;

    void setStencilRef(uint32_t ref) override;

    void dispatch(uint32_t num_groups_x,
                  uint32_t num_groups_y,
                  uint32_t num_groups_z) override;

    // @TODO: remove these
    void setMesh(const GpuMesh* mesh) override;

    void bindTexture(Dimension dimension, uint64_t handle, int slot) override;
    void unbindTexture(Dimension dimension, int slot) override;

    void bindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) override;
    void unbindStructuredBuffer(int slot) override;
    void bindStructuredBufferSRV(int slot, const GpuStructuredBuffer* buffer) override;
    void unbindStructuredBufferSRV(int slot) override;

    void bindUnorderedAccessView(uint32_t slot, GpuTexture* texture) override;
    void unbindUnorderedAccessView(uint32_t slot) override;

private:
    void setBlendState(const BlendDesc& desc, const float* factor, uint32_t mask);

    // @TODO: refactor
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
