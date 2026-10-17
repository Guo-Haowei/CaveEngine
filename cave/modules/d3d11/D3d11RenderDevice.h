#pragma once
#include <d3d11_1.h>
#include <wrl/client.h>

#include "D3d11CommandList.h"
#include "D3d11Resources.h"

#include "engine/private/core/base/rid_owner.h"
#include "engine/private/render/rhi/RenderDevice.h"

namespace cave::render {

class D3d11ViewCache;
struct D3d11MeshBuffers;

class D3d11RenderDevice final : public RenderDevice {
public:
    D3d11RenderDevice();

    void FinalizeImpl() override;

    auto createBuffer(const GpuResourceDesc& desc) -> Result<Ref<GpuResource>> override;

    auto createMeshImpl(const GpuMeshDesc& desc,
                        std::span<const GpuResourceDesc> vb_descs,
                        const GpuResourceDesc* ib_desc) -> Result<Ref<GpuMesh>> override;

    auto createConstantBuffer(const GpuResourceDesc& desc) -> Result<Ref<GpuResource>> override;
    auto createStructuredBuffer(const GpuResourceDesc& desc) -> Result<Ref<GpuResource>> override;

    void generateMipmap(const GpuResource* texture) override;

    // For fast and dirty access to device and device context, try not to use it
    Microsoft::WRL::ComPtr<ID3D11Device>& GetD3dDevice() { return m_device; }
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>& GetD3dContext() { return m_device_context; }

    // @TODO: refactor
    virtual ICommandList* getCommandList() const { return m_command_list.get(); }

    void updateConstantBuffer(const GpuResource* buffer, const void* data, size_t size) override;

    void updateBuffer(const GpuResourceDesc& desc, GpuResource* buffer) override;

protected:
    virtual auto InitializeInternal() -> Result<void> override;
    virtual Ref<GpuResource> createTextureImpl(const GpuResourceDesc& texture_desc, const SamplerDesc& sampler_desc) override;

    virtual void render() override;
    virtual void present() override;

    void onWindowResize(int width, int height) override;

    auto createDevice() -> Result<void>;
    auto createSwapChain() -> Result<void>;
    auto createRenderTarget() -> Result<void>;
    auto createSampler(uint32_t slot, D3D11_SAMPLER_DESC desc) -> Result<void>;
    auto initSamplers() -> Result<void>;

    Owner<D3d11ViewCache> m_view_cache;
    Owner<D3d11CommandList> m_command_list;

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_device_context;

    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_windowRtv;
    Microsoft::WRL::ComPtr<IDXGIDevice> m_dxgiDevice;
    Microsoft::WRL::ComPtr<IDXGIAdapter> m_dxgiAdapter;
    Microsoft::WRL::ComPtr<IDXGIFactory> m_dxgiFactory;
    Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> m_annotation;
    std::vector<Microsoft::WRL::ComPtr<ID3D11SamplerState>> m_samplers;

    RIDAllocator<D3d11MeshBuffers> m_meshes;
};

}  // namespace cave::render
