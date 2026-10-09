#pragma once
#include <d3d11_1.h>
#include <wrl/client.h>

#include "engine/private/core/base/rid_owner.h"
#include "engine/private/render/render_device/RenderDevice.h"

namespace cave::render {

class D3D11ViewCache;

struct D3d11Buffer : GpuBuffer {
    using GpuBuffer::GpuBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;

    uint64_t GetHandle() const final {
        return (size_t)buffer.Get();
    }
};

struct D3d11MeshBuffers : GpuMesh {
    using GpuMesh::GpuMesh;
};

class D3d11GraphicsManager : public RenderDevice {
public:
    D3d11GraphicsManager();

    void FinalizeImpl() final;

    void setStencilRef(uint32_t p_ref) final;
    void setBlendState(const BlendDesc& p_desc, const float* p_factor, uint32_t p_mask) final;

    void setRenderTargets(const RenderTargetDesc& p_target) final;
    void unsetRenderTargets() final;

    void clear(const RenderTargetDesc& p_target) final;

    void setViewport(const Viewport& p_viewport) final;

    auto createBuffer(const GpuBufferDesc& p_desc) -> Result<Ref<GpuBuffer>> final;

    auto createMeshImpl(const GpuMeshDesc& p_desc,
                        std::span<const GpuBufferDesc> p_vb_descs,
                        const GpuBufferDesc* p_ib_desc) -> Result<Ref<GpuMesh>> final;

    void setMesh(const GpuMesh* p_mesh) final;
    void updateBuffer(const GpuBufferDesc& p_desc, GpuBuffer* p_buffer) final;

    void drawElements(uint32_t p_count, uint32_t p_offset) final;
    void drawElementsInstanced(uint32_t p_instance_count, uint32_t p_count, uint32_t p_offset) final;
    void drawArrays(uint32_t p_count, uint32_t p_offset) final;
    void drawArraysInstanced(uint32_t p_instance_count, uint32_t p_count, uint32_t p_offset) final;

    void dispatch(uint32_t p_num_groups_x, uint32_t p_num_groups_y, uint32_t p_num_groups_z) final;
    void bindUnorderedAccessView(uint32_t p_slot, GpuTexture* p_texture) final;
    void unbindUnorderedAccessView(uint32_t p_slot) final;

    auto createConstantBuffer(const GpuBufferDesc& p_desc) -> Result<Ref<GpuConstantBuffer>> final;
    auto createStructuredBuffer(const GpuBufferDesc& p_desc) -> Result<Ref<GpuStructuredBuffer>> final;

    void bindStructuredBuffer(int p_slot, const GpuStructuredBuffer* p_buffer) final;
    void unbindStructuredBuffer(int p_slot) final;
    void bindStructuredBufferSRV(int p_slot, const GpuStructuredBuffer* p_buffer) final;
    void unbindStructuredBufferSRV(int p_slot) final;

    void updateConstantBuffer(const GpuConstantBuffer* p_buffer, const void* p_data, size_t p_size) final;
    void bindConstantBufferRange(const GpuConstantBuffer* p_buffer, uint32_t p_size, uint32_t p_offset) final;

    void bindTexture(Dimension p_dimension, uint64_t p_handle, int p_slot) final;
    void unbindTexture(Dimension p_dimension, int p_slot) final;

    void generateMipmap(const GpuTexture* p_texture) final;

    void beginEvent(std::string_view p_event) final;
    void endEvent() final;

    // For fast and dirty access to device and device context, try not to use it
    Microsoft::WRL::ComPtr<ID3D11Device>& GetD3dDevice() { return m_device; }
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>& GetD3dContext() { return m_deviceContext; }

protected:
    virtual auto InitializeInternal() -> Result<void> final;
    virtual Ref<GpuTexture> createTextureImpl(const GpuTextureDesc& p_texture_desc, const SamplerDesc& p_sampler_desc) final;

    virtual void render() final;
    virtual void present() final;

    void onWindowResize(int p_width, int p_height) final;
    void setPipelineStateImpl(PipelineStateName p_name) final;

    auto createDevice() -> Result<void>;
    auto createSwapChain() -> Result<void>;
    auto createRenderTarget() -> Result<void>;
    auto createSampler(uint32_t p_slot, D3D11_SAMPLER_DESC p_desc) -> Result<void>;
    auto initSamplers() -> Result<void>;

    Owner<D3D11ViewCache> m_view_cache;

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_windowRtv;
    Microsoft::WRL::ComPtr<IDXGIDevice> m_dxgiDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> m_dxgiAdapter;
    Microsoft::WRL::ComPtr<IDXGIFactory> m_dxgiFactory;
    Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> m_annotation;
    std::vector<Microsoft::WRL::ComPtr<ID3D11SamplerState>> m_samplers;

    RIDAllocator<D3d11MeshBuffers> m_meshes;

    // @TODO: cache
    struct {
        ID3D11RasterizerState* rasterizer = nullptr;
        ID3D11DepthStencilState* depthStencil = nullptr;
        uint32_t stencilRef = 0xFFFFFFFF;
        ID3D11BlendState* blendState = nullptr;
    } m_stateCache;
};

}  // namespace cave::render
