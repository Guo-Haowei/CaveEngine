#pragma once

#include <array>

#include "cave/render/rhi/ICommandList.h"
#include "engine/private/renderer/graphics_defines.h"

namespace cave::render {

struct PipelineState;
class MetalPipelineStateManager;

class MetalCommandList final : public ICommandList {
public:
    MetalCommandList(MetalPipelineStateManager& pso_manager, std::array<void*, 32>& samplers, Vector<void*>& constant_buffers) : m_pipeline_state_manager(pso_manager), m_samplers(samplers), m_constant_buffers(constant_buffers) {}

    void setCommandBuffer(void* command_buffer);
    void resetCommandBuffer();

    // Transitional helpers.
    // Move these into ICommandList later.

    void generateMipmap(const GpuTexture* texture);

    // ICommandList
    void draw(uint32_t count, uint32_t offset) override;
    void drawInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) override;
    void drawIndexed(uint32_t count, uint32_t offset) override;
    void drawIndexedInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) override;

    void beginRendering(const RenderingInfo& info) override;
    void endRendering() override;

    void pushDebugGroup(std::string_view name) override;
    void popDebugGroup() override;

    void bindConstantBufferRange(const GpuConstantBuffer* buffer, uint32_t size, uint32_t offset) override;

    void setViewport(const Viewport& viewport) override;
    void setPipelineState(PipelineStateName name) override;
    void setStencilRef(uint32_t ref) override;

    void dispatch(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) override;

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
    void applyGraphicsPipeline();
    void applyMesh();

    // Non-owning. RenderDevice owns the frame command buffer.
    void* m_command_buffer{};

    // Owned while an encoder is active.
    void* m_encoder{};

    const GpuMesh* m_current_mesh{};
    PipelineState* m_current_pso{};

    uint32_t m_stencil_ref{};

    std::array<uint64_t, 32> m_bound_textures{};
    std::array<uint64_t, 32> m_bound_uavs{};

    MetalPipelineStateManager& m_pipeline_state_manager;

    std::array<void*, 32>& m_samplers;
    Vector<void*>& m_constant_buffers;
};

} // namespace cave::render
