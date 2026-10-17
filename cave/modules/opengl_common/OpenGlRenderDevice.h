#pragma once

#include "engine/private/core/base/rid_owner.h"
#include "engine/private/render/rhi/RenderDevice.h"

#include "OpenGlDefines.h"

struct GLFWwindow;

namespace cave::render {

class OpenGlCommandList;
class OpenGlFramebufferCache;

// @TODO: fix
class OpenGlRenderDevice : public RenderDevice {
public:
    OpenGlRenderDevice();
    ~OpenGlRenderDevice();

    void FinalizeImpl() override;

    auto createBuffer(const GpuResourceDesc& desc) -> Result<Ref<GpuResource>> override;
    void updateBuffer(const GpuResourceDesc& desc, GpuResource* buffer) override;

    auto createMeshImpl(const GpuMeshDesc& desc,
                        std::span<const GpuResourceDesc> vb_descs,
                        const GpuResourceDesc* ib_desc) -> Result<Ref<GpuMesh>> override;

    auto createConstantBuffer(const GpuResourceDesc& desc) -> Result<Ref<GpuResource>> override;
    auto createStructuredBuffer(const GpuResourceDesc& desc) -> Result<Ref<GpuResource>> override;
    void updateBufferData(const GpuResourceDesc& desc, const GpuResource* buffer) override;

    void updateConstantBuffer(const GpuResource* buffer, const void* data, size_t size) override;

    void generateMipmap(const GpuResource* texture) override;

    // @TODO: refactor
    ICommandList* getCommandList() const override;

protected:
    Ref<GpuResource> createTextureImpl(const GpuResourceDesc& texture_desc, const SamplerDesc& sampler_desc) override;

    void render() override;
    void present() override;

    void onWindowResize(int, int) override {}

    GLFWwindow* m_window;

    struct {
        CullMode cullMode;
        bool frontCounterClockwise;
        ComparisonFunc depthFunc;
        bool enableDepthTest;
        bool enableStencilTest;
        ComparisonFunc stencilFunc;
        gl::TOPOLOGY topology;
    } m_stateCache;

private:
    Owner<OpenGlCommandList> m_command_list;
    Owner<OpenGlFramebufferCache> m_fbo_cache;
};

}  // namespace cave::render
