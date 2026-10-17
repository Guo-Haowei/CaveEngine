#pragma once

#include "engine/private/render/rhi/RenderDevice.h"

#include "MetalResources.h"

struct GLFWwindow;

namespace cave::render {

class MetalCommandList;

class MetalRenderDevice final : public RenderDevice {
public:
    MetalRenderDevice();
    ~MetalRenderDevice() override;

    void FinalizeImpl() override;

    void generateMipmap(const GpuResource* texture) override;

    //-----------------------------------------------------------------------------------------------------------------
    // Resources
    //-----------------------------------------------------------------------------------------------------------------

    auto createBuffer(const GpuResourceDesc& desc) -> Result<Ref<GpuResource>> override;

    void updateBuffer(const GpuResourceDesc& desc, GpuResource* buffer) override;

    auto createMeshImpl(const GpuMeshDesc& desc, std::span<const GpuResourceDesc> vb_descs, const GpuResourceDesc* ib_desc) -> Result<Ref<GpuMesh>> override;

    auto createConstantBuffer(const GpuResourceDesc& desc) -> Result<Ref<GpuResource>> override;

    auto createStructuredBuffer(const GpuResourceDesc& desc) -> Result<Ref<GpuResource>> override;

    void updateBufferData(const GpuResourceDesc& desc, const GpuResource* buffer) override;

    void updateConstantBuffer(const GpuResource* buffer, const void* data, size_t size) override;

    ICommandList* getCommandList() const override;

protected:
    auto InitializeInternal() -> Result<void> override;

    Ref<GpuResource> createTextureImpl(const GpuResourceDesc& desc, const SamplerDesc& sampler) override;

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
