#pragma once
#include <vulkan/vulkan.h>

#include "engine/private/render/render_device/RenderDevice.h"

struct GLFWwindow;

namespace cave::render {

WARNING_PUSH()
WARNING_DISABLE(4100, "-Wunused-parameter")

class VulkanRenderDevice : public RenderDevice {
public:
    VulkanRenderDevice();

    void FinalizeImpl() final;

    void setStencilRef(uint32_t ref) override {}

    void setRenderTargets(const RenderTargetDesc& target) override {}
    void unsetRenderTargets() override {}

    void clear(const RenderTargetDesc& target) override {}
    void setViewport(const Viewport& viewport) override {}

    auto createMeshImpl(const GpuMeshDesc& desc,
                        std::span<const GpuBufferDesc> vb_descs,
                        const GpuBufferDesc* ib_desc) -> Result<Ref<GpuMesh>> final {
        return nullptr;
    }

    auto createBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuBuffer>> override {
        return nullptr;
    }

    void setMesh(const GpuMesh* mesh) override {}

    void drawElements(uint32_t count, uint32_t offset) override {}
    void drawElementsInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) override {}
    void drawArrays(uint32_t count, uint32_t offset) override {}
    void drawArraysInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) override {}

    void dispatch(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) override {}
    void bindUnorderedAccessView(uint32_t slot, GpuTexture* texture) override {}
    void unbindUnorderedAccessView(uint32_t slot) override {}

    auto createConstantBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuConstantBuffer>> override { return nullptr; }
    auto createStructuredBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuStructuredBuffer>> override { return nullptr; }

    void bindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) override {}
    void unbindStructuredBuffer(int slot) override {}
    void bindStructuredBufferSRV(int slot, const GpuStructuredBuffer* buffer) override {}
    void unbindStructuredBufferSRV(int slot) override {}

    void updateConstantBuffer(const GpuConstantBuffer* buffer, const void* data, size_t size) override {}
    void bindConstantBufferRange(const GpuConstantBuffer* buffer, uint32_t size, uint32_t offset) override {}

    void bindTexture(Dimension dimension, uint64_t handle, int slot) override {}
    void unbindTexture(Dimension dimension, int slot) override {}

    void generateMipmap(const GpuTexture* texture) override {}

protected:
    auto InitializeInternal() -> Result<void> final;
    Ref<GpuTexture> createTextureImpl(const GpuTextureDesc& texture_desc, const SamplerDesc& sampler_desc) override { return nullptr; }

    void render() override {}
    void present() final;

    void onWindowResize(int width, int height) final;
    void setPipelineStateImpl(PipelineStateName name) override {}

private:
    auto createInstance() -> Result<void>;
    auto selectHardware() -> Result<void>;
    auto createDescriptorPool() -> Result<void>;

    GLFWwindow* m_window;
    VkSurfaceKHR m_surface;
};

WARNING_POP()

}  // namespace cave::render
