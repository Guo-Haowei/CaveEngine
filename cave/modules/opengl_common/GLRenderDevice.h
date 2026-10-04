#pragma once

#include "engine/private/core/base/rid_owner.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "GLDefines.h"

struct GLFWwindow;

namespace cave::render {

class GLFramebufferCache;

// @TODO: fix
struct OpenGlMeshBuffers : GpuMesh {
    using GpuMesh::GpuMesh;

    uint32_t vao{ 0 };
};

class GLRenderDevice : public RenderDevice {
public:
    GLRenderDevice();
    ~GLRenderDevice();

    void FinalizeImpl() override;

    void SetStencilRef(uint32_t ref) override;
    void SetBlendState(const BlendDesc& desc, const float* factor, uint32_t mask) override;

    void SetRenderTargets(const RenderTargetDesc& target) override;
    void UnsetRenderTargets() override;

    void Clear(const RenderTargetDesc& target) override;

    void SetViewport(const Viewport& viewport) override;

    auto CreateBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuBuffer>> override;
    void UpdateBuffer(const GpuBufferDesc& desc, GpuBuffer* buffer) override;

    auto CreateMeshImpl(const GpuMeshDesc& desc,
                        std::span<const GpuBufferDesc> vb_descs,
                        const GpuBufferDesc* ib_desc) -> Result<Ref<GpuMesh>> override;

    void SetMesh(const GpuMesh* mesh) override;

    void DrawElements(uint32_t count, uint32_t offset) override;
    void DrawElementsInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) override;
    void DrawArrays(uint32_t count, uint32_t offset) override;
    void DrawArraysInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) override;

    void Dispatch(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) override;
    void BindUnorderedAccessView(uint32_t slot, GpuTexture* texture) override;
    void UnbindUnorderedAccessView(uint32_t slot) override;

    void BindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) override;
    void UnbindStructuredBuffer(int slot) override;
    void BindStructuredBufferSRV(int slot, const GpuStructuredBuffer* buffer) override;
    void UnbindStructuredBufferSRV(int slot) override;

    auto CreateConstantBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuConstantBuffer>> override;
    auto CreateStructuredBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuStructuredBuffer>> override;
    void UpdateBufferData(const GpuBufferDesc& desc, const GpuStructuredBuffer* buffer) override;

    void UpdateConstantBuffer(const GpuConstantBuffer* buffer, const void* data, size_t size) override;
    void BindConstantBufferRange(const GpuConstantBuffer* buffer, uint32_t size, uint32_t offset) override;

    void BindTexture(Dimension dimension, uint64_t handle, int slot) override;
    void UnbindTexture(Dimension dimension, int slot) override;

    void GenerateMipmap(const GpuTexture* texture) override;

protected:
    Ref<GpuTexture> CreateTextureImpl(const GpuTextureDesc& texture_desc, const SamplerDesc& sampler_desc) override;

    void Render() override;
    void Present() override;

    void OnWindowResize(int, int) override {}
    void SetPipelineStateImpl(PipelineStateName name) override;

    // @TODO: rename
    RIDAllocator<OpenGlMeshBuffers> m_meshes;

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
    uint32_t m_dummy_vao;  // for drawing with gl_VertexID

    Owner<GLFramebufferCache> m_fbo_cache;
};

}  // namespace cave::render
