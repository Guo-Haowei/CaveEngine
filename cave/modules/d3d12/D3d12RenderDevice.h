#pragma once
#include "D3d12Core.h"
#include "engine/private/core/base/rid_owner.h"
#include "engine/private/render/render_device/RenderDevice.h"

namespace cave::render {

struct D3d12GpuTexture;
class D3d12ViewCache;

struct D3d12Buffer : GpuBuffer {
    using GpuBuffer::GpuBuffer;

    Microsoft::WRL::ComPtr<ID3D12Resource> buffer;

    uint64_t GetHandle() const final {
        return (size_t)buffer.Get();
    }
};

struct D3d12MeshBuffers : GpuMesh {
    using GpuMesh::GpuMesh;

    D3D12_VERTEX_BUFFER_VIEW vbvs[MESH_MAX_VERTEX_BUFFER_COUNT];
    D3D12_INDEX_BUFFER_VIEW ibv;
};

class D3d12RenderDevice : public RenderDevice {
public:
    D3d12RenderDevice();

    void FinalizeImpl() final;

    void setStencilRef(uint32_t ref) final;

    void setRenderTargets(const RenderTargetDesc& desc) final;
    void unsetRenderTargets() final;

    void clear(const RenderTargetDesc& target) final;

    void setViewport(const Viewport& viewport) final;

    auto createBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuBuffer>> final;
    void updateBuffer(const GpuBufferDesc& desc, GpuBuffer* buffer) final;

    auto createMeshImpl(const GpuMeshDesc& desc,
                        std::span<const GpuBufferDesc> vb_descs,
                        const GpuBufferDesc* ib_desc) -> Result<Ref<GpuMesh>> final;

    void setMesh(const GpuMesh* mesh) final;

    void drawElements(uint32_t count, uint32_t offset) final;
    void drawElementsInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) final;
    void drawArrays(uint32_t count, uint32_t offset) final;
    void drawArraysInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) final;

    void dispatch(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z) final;
    void bindUnorderedAccessView(uint32_t slot, GpuTexture* texture) final;
    void unbindUnorderedAccessView(uint32_t slot) final;

    auto createStructuredBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuStructuredBuffer>> final;
    void bindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) final;
    void unbindStructuredBuffer(int slot) final;
    void bindStructuredBufferSRV(int slot, const GpuStructuredBuffer* buffer) final;
    void unbindStructuredBufferSRV(int slot) final;

    auto createConstantBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuConstantBuffer>> final;
    void updateConstantBuffer(const GpuConstantBuffer* buffer, const void* data, size_t size) final;
    void bindConstantBufferRange(const GpuConstantBuffer* buffer, uint32_t size, uint32_t offset) final;

    // @TODO: remove Dimension
    void bindTexture(Dimension dimension, uint64_t handle, int slot) final;
    void unbindTexture(Dimension dimension, int slot) final;

    void generateMipmap(const GpuTexture* texture) final;

    ID3D12CommandQueue* createCommandQueue(D3D12_COMMAND_LIST_TYPE type);

    ID3D12Device4* const GetDevice() const { return m_device.Get(); }
    ID3D12RootSignature* const GetRootSignature() const { return m_root_signature.Get(); }

protected:
    auto InitializeInternal() -> Result<void> final;
    Ref<GpuTexture> createTextureImpl(const GpuTextureDesc& texture_desc, const SamplerDesc& sampler_desc) final;

    void render() final;
    void present() final;

    void beginFrame() final;
    void endFrame() final;
    void moveToNextFrame() final;
    Ref<FrameContext> createFrameContext() final;

    void beginPass(const CompiledPass& pass) final;
    void endPass(const CompiledPass& pass) final;

    void onWindowResize(int width, int height) final;
    void setPipelineStateImpl(PipelineStateName name) final;

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
    void TransitionTexture(D3d12GpuTexture* texture, D3D12_RESOURCE_STATES target_state);

    // @TODO: get rid of magic numbers
    DescriptorHeap m_rtvDescHeap;
    DescriptorHeap m_dsvDescHeap;
    DescriptorHeapSrv m_srvDescHeap;

    Microsoft::WRL::ComPtr<ID3D12Device4> m_device;
    Microsoft::WRL::ComPtr<ID3D12Debug> m_debug_controller;
    Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swap_chain;

    // Graphics Queue
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_graphicsCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_graphicsCommandList;
    Microsoft::WRL::ComPtr<ID3D12Fence1> m_graphicsQueueFence;

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

    Owner<D3d12ViewCache> m_view_cache;
};

}  // namespace cave::render
