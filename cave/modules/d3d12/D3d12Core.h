#pragma once
#include <d3d12.h>
#include <d3dx12/d3dx12.h>
#include <dxgi1_6.h>

#include <atomic>
#include <mutex>

#include "../d3d_common/D3dCommon.h"
#include "engine/private/render/rhi/RenderDevice.h"

namespace cave::render {

class D3d12RenderDevice;

struct DescriptorHeapHandle {
    int index;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle;
};

class DescriptorHeap {
public:
    virtual ~DescriptorHeap() = default;

    virtual auto Initialize(int count,
                            D3D12_DESCRIPTOR_HEAP_TYPE type,
                            ID3D12Device* device,
                            bool shard_visible) -> Result<void>;

    D3D12_CPU_DESCRIPTOR_HANDLE GetStartCpu() const { return m_start_cpu; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetStartGpu() const { return m_start_gpu; };
    ID3D12DescriptorHeap* GetHeap() { return m_heap.Get(); }

    uint32_t GetIncrementSize() const { return m_increment_size; }

    DescriptorHeapHandle AllocHandle();

protected:
    D3D12_DESCRIPTOR_HEAP_DESC m_desc{};
    D3D12_CPU_DESCRIPTOR_HANDLE m_start_cpu{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_start_gpu{};
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap{};

    uint32_t m_increment_size{ 0 };
    int m_capacity{ 0 };
    std::atomic_int m_counter{ 0 };
};

class DescriptorHeapSrv : public DescriptorHeap {
public:
    using DescriptorHeap::DescriptorHeap;

    virtual auto Initialize(int count,
                            D3D12_DESCRIPTOR_HEAP_TYPE type,
                            ID3D12Device* device,
                            bool shard_visible) -> Result<void> override;
};

// @TODO: refactor, this is duplicate
struct GPUBufferDesc {
    uint32_t byteWidth = 0;
};

struct GPUBuffer {
    GPUBufferDesc desc;
    Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
};

class CopyContext {
public:
    struct CopyCommand {
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
        GPUBuffer uploadBuffer;
        void* data = nullptr;
        // ID3D12Resource* uploadResource = nullptr;
    };

    auto Initialize(D3d12RenderDevice* p_device) -> Result<void>;

    void Finalize();

    CopyCommand Allocate(uint32_t p_staging_size);

    void submit(CopyCommand p_cmd);

private:
    D3d12RenderDevice* m_device = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_queue;
    std::vector<CopyCommand> m_freeList;
    std::mutex m_lock;
};

}  // namespace cave::render