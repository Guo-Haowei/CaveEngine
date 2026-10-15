#pragma once
#include "D3d12Core.h"
#include "D3d12Resources.h"

#include "engine/private/core/base/rid_owner.h"
#include "engine/private/render/render_device/RenderDevice.h"

namespace cave::render {

struct D3d12GpuTexture;
class D3d12CommandList;
class D3d12ViewCache;

struct D3d12Buffer final : GpuBuffer {
    using GpuBuffer::GpuBuffer;

    Microsoft::WRL::ComPtr<ID3D12Resource> buffer;

    uint64_t GetHandle() const override {
        return (size_t)buffer.Get();
    }
};

class D3d12RenderDevice final : public RenderDevice {
public:
    D3d12RenderDevice();

    void FinalizeImpl() override;

    void setRenderTargets(const RenderTargetDesc& desc) override;
    void unsetRenderTargets() override;

    void setViewport(const Viewport& viewport) override;

    auto createBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuBuffer>> override;
    void updateBuffer(const GpuBufferDesc& desc, GpuBuffer* buffer) override;

    auto createMeshImpl(const GpuMeshDesc& desc,
                        std::span<const GpuBufferDesc> vb_descs,
                        const GpuBufferDesc* ib_desc) -> Result<Ref<GpuMesh>> override;

    void setMesh(const GpuMesh* mesh) override;

    void bindUnorderedAccessView(uint32_t slot, GpuTexture* texture) override;
    void unbindUnorderedAccessView(uint32_t slot) override;

    auto createStructuredBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuStructuredBuffer>> override;

    auto createConstantBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuConstantBuffer>> override;
    void updateConstantBuffer(const GpuConstantBuffer* buffer, const void* data, size_t size) override;

    // @TODO: remove Dimension
    void generateMipmap(const GpuTexture* texture) override;

    ID3D12CommandQueue* createCommandQueue(D3D12_COMMAND_LIST_TYPE type);

    ID3D12Device4* const getDevice() const { return m_device.Get(); }
    ID3D12RootSignature* const getRootSignature() const { return m_root_signature.Get(); }

    ICommandList* getCommandList() const override;

protected:
    auto InitializeInternal() -> Result<void> override;
    Ref<GpuTexture> createTextureImpl(const GpuTextureDesc& texture_desc, const SamplerDesc& sampler_desc) override;

    void render() override;
    void present() override;

    void beginFrame() override;
    void endFrame() override;
    void moveToNextFrame() override;
    Ref<FrameContext> createFrameContext() override;

    void beginPass(const CompiledPass& pass) override;
    void endPass(const CompiledPass& pass) override;

    void onWindowResize(int width, int height) override;

private:
    auto createDevice() -> Result<void>;
    auto initGraphicsContext() -> Result<void>;
    void finalizeGraphicsContext();
    void flushGraphicsContext();

    auto enableDebugLayer() -> Result<void>;
    auto createDescriptorHeaps() -> Result<void>;
    auto createRootSignature() -> Result<void>;
    auto createSwapChain(uint32_t width, uint32_t height) -> Result<void>;
    auto createRenderTarget(uint32_t width, uint32_t height) -> Result<void>;
    void cleanupRenderTarget();
    void initStaticSamplers();

    ID3D12Resource* uploadBuffer(uint32_t byte_size, const void* init_data, ID3D12Resource* out_buffer);
    void transitionTexture(D3d12GpuTexture* texture, D3D12_RESOURCE_STATES target_state);

    DescriptorHeap m_rtv_heap;
    DescriptorHeap m_dsv_heap;
    DescriptorHeapSrv m_srv_heap;

    Microsoft::WRL::ComPtr<ID3D12Device4> m_device;
    Microsoft::WRL::ComPtr<ID3D12Debug> m_debug_controller;
    Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swap_chain;

    // Graphics Queue
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_graphics_command_queue;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_graphics_command_list;
    Microsoft::WRL::ComPtr<ID3D12Fence1> m_graphics_queue_fence;

    uint64_t m_lastSignaledFenceValue = 0;
    HANDLE m_graphicsFenceEvent = NULL;

    // Copy Queue
    CopyContext m_copyContext;

    HANDLE m_swapChainWaitObject = INVALID_HANDLE_VALUE;

    // Render Target

    ID3D12Resource* m_renderTargets[NUM_BACK_BUFFERS]{ nullptr };
    D3D12_CPU_DESCRIPTOR_HANDLE m_renderTargetDescriptor[NUM_BACK_BUFFERS]{};
    ID3D12Resource* m_depthStencilBuffer{ nullptr };
    D3D12_CPU_DESCRIPTOR_HANDLE m_depthStencilDescriptor{};

    uint32_t m_backbufferIndex = 0;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_root_signature;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_debugVertexData;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_debugIndexData;
    Vector<CD3DX12_STATIC_SAMPLER_DESC> m_staticSamplers;

    Vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_textures;

    RIDAllocator<D3d12MeshBuffers> m_meshes;

    Owner<D3d12CommandList> m_command_list;
    Owner<D3d12ViewCache> m_view_cache;
};

}  // namespace cave::render
