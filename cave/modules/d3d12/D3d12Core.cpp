#include "D3d12Core.h"

#include "../d3d_common/D3dCommon.h"
#include "D3d12RenderDevice.h"

namespace cave::render {

using namespace cave::math;

static constexpr int DESCRIPTOR_MAX_COUNT = 512;

//------------------------------------------------------------------------------
// DescriptorHeap
auto DescriptorHeap::Initialize(int count, D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12Device* device, bool shader_visible) -> Result<void> {
    m_desc.Type = type;
    m_desc.NumDescriptors = count;
    m_desc.Flags = shader_visible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    m_desc.NodeMask = 1;
    m_capacity = count;

    std::string_view name = "";
    switch (type) {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            name = "CBV_SRV_UAV";
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            name = "RTV Heap";
            break;
        case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            name = "DSV Heap";
            break;
        default:
            CRASH_NOW();
            break;
    }

    D3D_FAIL(device->CreateDescriptorHeap(&m_desc, IID_PPV_ARGS(&m_heap)), "Failed to create heap {}", name);

    D3D12_SET_DEBUG_NAME(m_heap.Get(), name);

    m_increment_size = device->GetDescriptorHandleIncrementSize(type);
    m_start_cpu = m_heap->GetCPUDescriptorHandleForHeapStart();
    if (shader_visible) {
        m_start_gpu = m_heap->GetGPUDescriptorHandleForHeapStart();
    } else {
        m_start_gpu.ptr = 0;
    }

    return Result<void>();
}

DescriptorHeapHandle DescriptorHeap::AllocHandle() {
    const int index = m_counter.fetch_add(1);
    DEV_ASSERT_MSG(index < m_capacity, "DescriptorHeap is out of capacity!");

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle(m_start_cpu);
    cpu_handle.Offset(index, m_increment_size);

    CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle(m_start_gpu);
    if (m_start_gpu.ptr != 0) {
        gpu_handle.Offset(index, m_increment_size);
    }

    return {
        index,
        cpu_handle,
        gpu_handle,
    };
}

//------------------------------------------------------------------------------
// DescriptorHeapSrv
//------------------------------------------------------------------------------

auto DescriptorHeapSrv::Initialize(int count,
                                   D3D12_DESCRIPTOR_HEAP_TYPE type,
                                   ID3D12Device* device,
                                   bool shader_visible) -> Result<void> {
    m_capacity = count;
    // reserve 1 for imgui
    m_counter = 1;

    return DescriptorHeap::Initialize(count, type, device, shader_visible);
}

//------------------------------------------------------------------------------
// CopyContext
//------------------------------------------------------------------------------
auto CopyContext::Initialize(D3d12RenderDevice* p_device) -> Result<void> {
    DEV_ASSERT(p_device);
    m_device = p_device;
    m_queue = p_device->createCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);

    if (DEV_VERIFY(m_queue)) {
        return Result<void>();
    }
    return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "failed to create command queue");
}

void CopyContext::Finalize() {
    m_queue.Reset();
    m_freeList.clear();
}

CopyContext::CopyCommand CopyContext::Allocate(uint32_t p_staging_size) {
    ID3D12Device4* const device = m_device->getDevice();
    m_lock.lock();
    // create a new command list if there are no free ones
    if (m_freeList.empty()) {
        CopyCommand cmd;
        D3D_CALL(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY,
                                                IID_PPV_ARGS(cmd.commandAllocator.GetAddressOf())));
        D3D_CALL(device->CreateCommandList1(0,
                                            D3D12_COMMAND_LIST_TYPE_COPY,
                                            D3D12_COMMAND_LIST_FLAG_NONE,
                                            IID_PPV_ARGS(cmd.commandList.GetAddressOf())));
        D3D_CALL(device->CreateFence(0,
                                     D3D12_FENCE_FLAG_NONE,
                                     IID_PPV_ARGS(cmd.fence.GetAddressOf())));
        m_freeList.emplace_back(cmd);
    }

    // Search for a staging buffer that can fit the request
    CopyCommand cmd = m_freeList.back();
    if (cmd.uploadBuffer.desc.byteWidth < p_staging_size) {
        for (size_t i = 0; i < m_freeList.size(); ++i) {
            if (m_freeList[i].uploadBuffer.desc.byteWidth >= p_staging_size) {
                cmd = m_freeList[i];
                std::swap(m_freeList[i], m_freeList.back());
                break;
            }
        }
    }
    m_freeList.pop_back();
    m_lock.unlock();

    // if no buffer was found, create one
    if (cmd.uploadBuffer.desc.byteWidth < p_staging_size) {
        if (cmd.uploadBuffer.buffer) {
            cmd.uploadBuffer.buffer.Reset();
        }

        cmd.uploadBuffer.desc.byteWidth = NextPowerOfTwo(p_staging_size);
        auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto buffer = CD3DX12_RESOURCE_DESC::Buffer(cmd.uploadBuffer.desc.byteWidth);
        D3D_CALL(device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &buffer,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(cmd.uploadBuffer.buffer.GetAddressOf())));
    }
    // else
    //{
    //     LOG("Copy buffer reused of size ", staging_size);
    // }

    // begine command list in valid state
    D3D_CALL(cmd.commandAllocator->Reset());
    D3D_CALL(cmd.commandList->Reset(cmd.commandAllocator.Get(), nullptr));
    return cmd;
}

void CopyContext::submit(CopyCommand p_cmd) {
    D3D_CALL(p_cmd.commandList->Close());
    ID3D12CommandList* commandLists[] = { p_cmd.commandList.Get() };
    m_queue->ExecuteCommandLists(std::size(commandLists), commandLists);
    D3D_CALL(m_queue->Signal(p_cmd.fence.Get(), 1));
    D3D_CALL(p_cmd.fence->SetEventOnCompletion(1, NULL));
    D3D_CALL(p_cmd.fence->Signal(0));

    m_lock.lock();
    m_freeList.push_back(p_cmd);
    m_lock.unlock();
}

}  // namespace cave::render
