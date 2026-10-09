#pragma once
#include <d3d11_1.h>
#include <wrl/client.h>

#include "engine/private/core/base/rid_owner.h"
#include "engine/private/render/render_device/RenderDevice.h"

namespace cave::render {

class D3d11ViewCache;

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

class D3d11RenderDevice : public RenderDevice {
public:
    D3d11RenderDevice();

    void FinalizeImpl() final;

    void setStencilRef(uint32_t ref) final;

    void setRenderTargets(const RenderTargetDesc& target) final;
    void unsetRenderTargets() final;

    void clear(const RenderTargetDesc& target) final;

    void setViewport(const Viewport& viewport) final;

    auto createBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuBuffer>> final;

    auto createMeshImpl(const GpuMeshDesc& desc,
                        std::span<const GpuBufferDesc> vb_descs,
                        const GpuBufferDesc* ib_desc) -> Result<Ref<GpuMesh>> final;

    void setMesh(const GpuMesh* mesh) final;
    void updateBuffer(const GpuBufferDesc& desc, GpuBuffer* buffer) final;

    void drawElements(uint32_t count, uint32_t offset) final;
    void drawElementsInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) final;
    void drawArrays(uint32_t count, uint32_t offset) final;
    void drawArraysInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) final;

    void dispatch(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) final;
    void bindUnorderedAccessView(uint32_t slot, GpuTexture* texture) final;
    void unbindUnorderedAccessView(uint32_t slot) final;

    auto createConstantBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuConstantBuffer>> final;
    auto createStructuredBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuStructuredBuffer>> final;

    void bindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) final;
    void unbindStructuredBuffer(int slot) final;
    void bindStructuredBufferSRV(int slot, const GpuStructuredBuffer* buffer) final;
    void unbindStructuredBufferSRV(int slot) final;

    void updateConstantBuffer(const GpuConstantBuffer* buffer, const void* data, size_t size) final;
    void bindConstantBufferRange(const GpuConstantBuffer* buffer, uint32_t size, uint32_t offset) final;

    void bindTexture(Dimension dimension, uint64_t handle, int slot) final;
    void unbindTexture(Dimension dimension, int slot) final;

    void generateMipmap(const GpuTexture* texture) final;

    void beginEvent(std::string_view event) final;
    void endEvent() final;

    // For fast and dirty access to device and device context, try not to use it
    Microsoft::WRL::ComPtr<ID3D11Device>& GetD3dDevice() { return m_device; }
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>& GetD3dContext() { return m_deviceContext; }

protected:
    virtual auto InitializeInternal() -> Result<void> final;
    virtual Ref<GpuTexture> createTextureImpl(const GpuTextureDesc& texture_desc, const SamplerDesc& sampler_desc) final;

    virtual void render() final;
    virtual void present() final;

    void onWindowResize(int width, int height) final;
    void setPipelineStateImpl(PipelineStateName name) final;

    auto createDevice() -> Result<void>;
    auto createSwapChain() -> Result<void>;
    auto createRenderTarget() -> Result<void>;
    auto createSampler(uint32_t slot, D3D11_SAMPLER_DESC desc) -> Result<void>;
    auto initSamplers() -> Result<void>;

    Owner<D3d11ViewCache> m_view_cache;

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
