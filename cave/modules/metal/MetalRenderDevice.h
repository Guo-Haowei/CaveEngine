#pragma once

#include "engine/private/render/render_device/RenderDevice.h"

#include "MetalResources.h"

struct GLFWwindow;

namespace cave::render {

class MetalCommandList;

class MetalRenderDevice final : public RenderDevice {
public:
    MetalRenderDevice();
    ~MetalRenderDevice() override;

    void FinalizeImpl() override;

    void generateMipmap(const GpuTexture* texture) override;

    //-----------------------------------------------------------------------------------------------------------------
    // Resources
    //-----------------------------------------------------------------------------------------------------------------

    auto createBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuBuffer>> override;

    void updateBuffer(const GpuBufferDesc& desc, GpuBuffer* buffer) override;

    auto createMeshImpl(const GpuMeshDesc& desc, std::span<const GpuBufferDesc> vb_descs, const GpuBufferDesc* ib_desc) -> Result<Ref<GpuMesh>> override;

    auto createConstantBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuConstantBuffer>> override;

    auto createStructuredBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuStructuredBuffer>> override;

    void updateBufferData(const GpuBufferDesc& desc, const GpuStructuredBuffer* buffer) override;

    void updateConstantBuffer(const GpuConstantBuffer* buffer, const void* data, size_t size) override;

    ICommandList* getCommandList() const override;

protected:
    auto InitializeInternal() -> Result<void> override;

    Ref<GpuTexture> createTextureImpl(const GpuTextureDesc& desc, const SamplerDesc& sampler) override;

    void beginFrame() override;
    void endFrame() override;

    void render() override;
    void present() override;

    void onWindowResize(int width, int height) override;

private:
    GLFWwindow* m_window{};

    void* m_device{};
    void* m_command_queue{};

    // Current frame command buffer.
    void* m_command_buffer{};

    void* m_layer{};
    void* m_drawable{};

    // Transitional global resource binding storage.
    std::array<void*, 32> m_samplers{};
    Vector<void*> m_constant_buffers{};

    Owner<MetalCommandList> m_command_list;
};

} // namespace cave::render
