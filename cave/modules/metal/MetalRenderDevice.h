#pragma once
#include "engine/private/render/render_device/RenderDevice.h"

struct GLFWwindow;

namespace cave::render {

struct MetalGpuMesh : GpuMesh {
    using GpuMesh::GpuMesh;
};

class MetalRenderDevice final : public RenderDevice {
public:
    MetalRenderDevice();
    ~MetalRenderDevice() override;

    void FinalizeImpl() override;
    void setStencilRef(uint32_t ref) override;
    void setBlendState(const BlendDesc& desc, const float* factor, uint32_t mask) override;
    void setRenderTargets(const RenderTargetDesc& target) override;
    void unsetRenderTargets() override;
    void clear(const RenderTargetDesc& target) override;
    void setViewport(const Viewport& viewport) override;

    auto createBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuBuffer>> override;
    void updateBuffer(const GpuBufferDesc& desc, GpuBuffer* buffer) override;
    auto createMeshImpl(const GpuMeshDesc& desc, std::span<const GpuBufferDesc> vb_descs, const GpuBufferDesc* ib_desc) -> Result<Ref<GpuMesh>> override;
    void setMesh(const GpuMesh* mesh) override;
    void drawElements(uint32_t count, uint32_t offset) override;
    void drawElementsInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) override;
    void drawArrays(uint32_t count, uint32_t offset) override;
    void drawArraysInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) override;
    void dispatch(uint32_t x, uint32_t y, uint32_t z) override;
    void bindUnorderedAccessView(uint32_t slot, GpuTexture* texture) override;
    void unbindUnorderedAccessView(uint32_t slot) override;
    void bindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) override;
    void unbindStructuredBuffer(int slot) override;
    void bindStructuredBufferSRV(int slot, const GpuStructuredBuffer* buffer) override;
    void unbindStructuredBufferSRV(int slot) override;
    auto createConstantBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuConstantBuffer>> override;
    auto createStructuredBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuStructuredBuffer>> override;
    void updateBufferData(const GpuBufferDesc& desc, const GpuStructuredBuffer* buffer) override;
    void updateConstantBuffer(const GpuConstantBuffer* buffer, const void* data, size_t size) override;
    void bindConstantBufferRange(const GpuConstantBuffer* buffer, uint32_t size, uint32_t offset) override;
    void bindTexture(Dimension dimension, uint64_t handle, int slot) override;
    void unbindTexture(Dimension dimension, int slot) override;
    void generateMipmap(const GpuTexture* texture) override;

protected:
    auto InitializeInternal() -> Result<void> override;
    Ref<GpuTexture> createTextureImpl(const GpuTextureDesc&, const SamplerDesc&) override;
    void beginFrame() override;
    void endFrame() override;
    void render() override;
    void present() override;
    void onWindowResize(int, int) override;
    void setPipelineStateImpl(PipelineStateName name) override;

private:
    void setupPipeline();
    void setupVertexBuffer();

    GLFWwindow* m_window{};
    void* m_device{};
    void* m_command_queue{};
    void* m_command_buffer{};
    void* m_layer{};
    void* m_encoder{};
    void* m_drawable{};
    void* m_pipeline{};
    void* m_default_sampler{};
    void* m_current_mesh{};
    uint32_t m_stencil_ref{};
    void* m_current_pso{};
    std::array<uint64_t, 32> m_bound_textures{};
    std::array<uint64_t, 32> m_bound_uavs{};
    std::array<void*, 32> m_samplers{};
    Vector<void*> m_constant_buffers{};
};

} // namespace cave::render
