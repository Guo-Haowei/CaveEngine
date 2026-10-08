#pragma once
#include "engine/private/render/render_device/RenderDevice.h"

struct GLFWwindow;

namespace cave::render {

WARNING_PUSH()
WARNING_DISABLE(4100, "-Wunused-parameter")

class MetalRenderDevice : public RenderDevice {
public:
    MetalRenderDevice();

    void setRenderTargets(const RenderTargetDesc&) override {}
    void unsetRenderTargets() override {}

    auto createBuffer(const GpuBufferDesc&) -> Result<Ref<GpuBuffer>> override { return nullptr; }
    void updateBuffer(const GpuBufferDesc&, GpuBuffer*) override {}

    auto createMesh(const MeshAsset&) -> Result<Ref<GpuMesh>> override { return nullptr; }
    auto createMeshImpl(const GpuMeshDesc&,
                        std::span<const GpuBufferDesc>,
                        const GpuBufferDesc*) -> Result<Ref<GpuMesh>> final {
        return nullptr;
    }

    void setStencilRef(uint32_t ref) override {}
    void setBlendState(const BlendDesc& desc, const float* factor, uint32_t mask) override {}

    void beginPass(const CompiledPass& pass) override {}
    void endPass(const CompiledPass& pass) override {}

    void clear(const RenderTargetDesc& target) override {}
    void setViewport(const Viewport& viewport) override {}

    void setMesh(const GpuMesh*) override {}

    void drawElements(uint32_t count, uint32_t offset) override {}
    void drawElementsInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) override {}

    void drawArrays(uint32_t, uint32_t) override {}
    void drawArraysInstanced(uint32_t, uint32_t, uint32_t) override {}

    void dispatch(uint32_t p_num_groups_x, uint32_t p_num_groups_y, uint32_t p_num_groups_z) override {}
    void bindUnorderedAccessView(uint32_t p_slot, GpuTexture* p_texture) override {}
    void unbindUnorderedAccessView(uint32_t p_slot) override {}

    auto createConstantBuffer(const GpuBufferDesc& p_desc) -> Result<std::shared_ptr<GpuConstantBuffer>> override { return nullptr; }
    auto createStructuredBuffer(const GpuBufferDesc& p_desc) -> Result<std::shared_ptr<GpuStructuredBuffer>> override { return nullptr; }

    void bindStructuredBuffer(int p_slot, const GpuStructuredBuffer* p_buffer) override {}
    void unbindStructuredBuffer(int p_slot) override {}
    void bindStructuredBufferSRV(int p_slot, const GpuStructuredBuffer* p_buffer) override {}
    void unbindStructuredBufferSRV(int p_slot) override {}

    void updateConstantBuffer(const GpuConstantBuffer* p_buffer, const void* p_data, size_t p_size) override {}
    void bindConstantBufferRange(const GpuConstantBuffer* p_buffer, uint32_t p_size, uint32_t p_offset) override {}

    void bindTexture(Dimension p_dimension, uint64_t p_handle, int p_slot) override {}
    void unbindTexture(Dimension p_dimension, int p_slot) override {}

    void generateMipmap(const GpuTexture* p_texture) override {}

protected:
    auto InitializeInternal() -> Result<void> final;
    void FinalizeImpl() final;

    Ref<GpuTexture> createTextureImpl(const GpuTextureDesc& p_texture_desc, const SamplerDesc& p_sampler_desc) final;

    void render() override {}
    void present() final;

    void onWindowResize(int width, int height) final;
    void setPipelineStateImpl(PipelineStateName name) override {}

private:
    void setupPipeline();
    void setupVertexBuffer();

    GLFWwindow* m_window;
    void* m_device;
    void* m_commandQueue;
    void* m_renderPassDescriptor;
    void* m_layer;
};

WARNING_POP()

} // namespace cave
