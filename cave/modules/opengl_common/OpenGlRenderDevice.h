#pragma once

#include "engine/private/core/base/rid_owner.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "OpenGlDefines.h"

struct GLFWwindow;

namespace cave::render {

class OpenGlFramebufferCache;

// @TODO: fix
struct OpenGlGpuMesh : GpuMesh {
    using GpuMesh::GpuMesh;

    uint32_t vao{ 0 };
};

class OpenGlRenderDevice : public RenderDevice {
public:
    OpenGlRenderDevice();
    ~OpenGlRenderDevice();

    void FinalizeImpl() override;

    void setStencilRef(uint32_t ref) override;

    void setRenderTargets(const RenderTargetDesc& target) override;
    void unsetRenderTargets() override;

    void clear(const RenderTargetDesc& target) override;

    void setViewport(const Viewport& viewport) override;

    auto createBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuBuffer>> override;
    void updateBuffer(const GpuBufferDesc& desc, GpuBuffer* buffer) override;

    auto createMeshImpl(const GpuMeshDesc& desc,
                        std::span<const GpuBufferDesc> vb_descs,
                        const GpuBufferDesc* ib_desc) -> Result<Ref<GpuMesh>> override;

    void setMesh(const GpuMesh* mesh) override;

    void drawElements(uint32_t count, uint32_t offset) override;
    void drawElementsInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) override;
    void drawArrays(uint32_t count, uint32_t offset) override;
    void drawArraysInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) override;

    void dispatch(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) override;
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
    Ref<GpuTexture> createTextureImpl(const GpuTextureDesc& texture_desc, const SamplerDesc& sampler_desc) override;

    void render() override;
    void present() override;

    void onWindowResize(int, int) override {}
    void setPipelineStateImpl(PipelineStateName name) override;

    // @TODO: rename
    RIDAllocator<OpenGlGpuMesh> m_meshes;

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
    void setBlendState(const BlendDesc& desc, const float* factor, uint32_t mask);

    uint32_t m_dummy_vao;  // for drawing with gl_VertexID

    Owner<OpenGlFramebufferCache> m_fbo_cache;
};

}  // namespace cave::render
