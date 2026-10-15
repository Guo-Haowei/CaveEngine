#pragma once

#include "engine/private/core/base/rid_owner.h"
#include "engine/private/render/render_device/RenderDevice.h"

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

    auto createBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuBuffer>> override;
    void updateBuffer(const GpuBufferDesc& desc, GpuBuffer* buffer) override;

    auto createMeshImpl(const GpuMeshDesc& desc,
                        std::span<const GpuBufferDesc> vb_descs,
                        const GpuBufferDesc* ib_desc) -> Result<Ref<GpuMesh>> override;

    auto createConstantBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuConstantBuffer>> override;
    auto createStructuredBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuStructuredBuffer>> override;
    void updateBufferData(const GpuBufferDesc& desc, const GpuStructuredBuffer* buffer) override;

    void updateConstantBuffer(const GpuConstantBuffer* buffer, const void* data, size_t size) override;

    void generateMipmap(const GpuTexture* texture) override;

    // @TODO: refactor
    ICommandList* getCommandList() const override;

protected:
    Ref<GpuTexture> createTextureImpl(const GpuTextureDesc& texture_desc, const SamplerDesc& sampler_desc) override;

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
