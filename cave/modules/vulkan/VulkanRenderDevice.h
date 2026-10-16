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

    auto createMeshImpl(const GpuMeshDesc& desc,
                        std::span<const GpuResourceDesc> vb_descs,
                        const GpuResourceDesc* ib_desc) -> Result<Ref<GpuMesh>> final {
        return nullptr;
    }

    auto createBuffer(const GpuResourceDesc& desc) -> Result<Ref<GpuBuffer>> override {
        return nullptr;
    }

    auto createConstantBuffer(const GpuResourceDesc& desc) -> Result<Ref<GpuBuffer>> override { return nullptr; }
    auto createStructuredBuffer(const GpuResourceDesc& desc) -> Result<Ref<GpuBuffer>> override { return nullptr; }

    void updateConstantBuffer(const GpuBuffer* buffer, const void* data, size_t size) override {}

    void generateMipmap(const GpuTexture* texture) override {}

    ICommandList* getCommandList() const override { return nullptr; }

protected:
    auto InitializeInternal() -> Result<void> final;
    Ref<GpuTexture> createTextureImpl(const GpuResourceDesc& texture_desc, const SamplerDesc& sampler_desc) override { return nullptr; }

    void render() override {}
    void present() final;

    void onWindowResize(int width, int height) final;

private:
    auto createInstance() -> Result<void>;
    auto selectHardware() -> Result<void>;
    auto createDescriptorPool() -> Result<void>;

    GLFWwindow* m_window;
    VkSurfaceKHR m_surface;
};

WARNING_POP()

}  // namespace cave::render
