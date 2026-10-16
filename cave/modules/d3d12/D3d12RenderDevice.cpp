#include "D3d12RenderDevice.h"

#include <imgui/backends/imgui_impl_dx12.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "D3d12CommandList.h"
#include "D3d12PipelineStateManager.h"
#include "D3d12Resources.h"
#include "D3d12ViewCache.h"

#include "cave/core/string/StringUtils.h"
#include "cave/render/rhi/Sampler.h"
#include "cave/runtime/framework/IApplication.h"

#include "../d3d_common/D3dCommon.h"
#include "engine/private/core/math/MatrixTransform.h"
#include "engine/private/runtime/display/GlfwDisplayService.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/scene/Scene.h"

#define INCLUDE_AS_D3D12
#include "../d3d_common/D3dConvert.h"

namespace cave {
#include "structured_buffer.slang.h"
}  // namespace cave

namespace cave::render {

using namespace cave::math;
using Microsoft::WRL::ComPtr;

struct D3d12FrameContext : FrameContext {
    ~D3d12FrameContext() {
        SafeRelease(m_commandAllocator);
    }

    void Wait(HANDLE p_fence_event, ID3D12Fence1* p_fence) {
        if (p_fence->GetCompletedValue() < m_fenceValue) {
            D3D_CALL(p_fence->SetEventOnCompletion(m_fenceValue, p_fence_event));
            WaitForSingleObject(p_fence_event, INFINITE);
        }
    }

    ID3D12CommandAllocator* m_commandAllocator = nullptr;
    uint64_t m_fenceValue = 0;
};

D3d12RenderDevice::D3d12RenderDevice()
    : RenderDevice("D3d12RenderDevice", Backend::D3d12, kNumFramesInFlight) {
    m_pipeline_state_manager = MakeOwner<D3d12PipelineStateManager>(this);
}

auto D3d12RenderDevice::InitializeInternal() -> Result<void> {
    const auto size = DisplayService::singleton().windowSize();
    const int w = size.x;
    const int h = size.y;
    DEV_ASSERT(w > 0 && h > 0);

    if (auto res = createDevice(); !res) {
        return CAVE_ERROR(res.error());
    }

    if (m_enable_validation_layer) {
        if (enableDebugLayer()) {
            LOG_INFO(LogChannel::RHI, "Debug layer enabled");
        } else {
            LOG_ERROR(LogChannel::RHI, "Debug layer not enabled");
        }
    }

    if (auto res = createDescriptorHeaps(); !res) {
        return CAVE_ERROR(res.error());
    }
    if (auto res = initGraphicsContext(); !res) {
        return CAVE_ERROR(res.error());
    }
    if (auto res = m_copyContext.Initialize(this); !res) {
        return CAVE_ERROR(res.error());
    }

    for (int i = 0; i < kNumBackBuffers; i++) {
        auto handle = m_rtv_heap.AllocHandle();
        m_renderTargetDescriptor[i] = handle.cpu_handle;
    }

    {
        auto handle = m_dsv_heap.AllocHandle();
        m_depthStencilDescriptor = handle.cpu_handle;
    }

    if (auto res = createSwapChain(w, h); !res) {
        return CAVE_ERROR(res.error());
    }
    if (auto res = createRenderTarget(w, h); !res) {
        return CAVE_ERROR(res.error());
    }
    if (auto res = createRootSignature(); !res) {
        return CAVE_ERROR(res.error());
    }

    m_view_cache = MakeOwner<D3d12ViewCache>(m_device.Get(), m_rtv_heap, m_dsv_heap);

    m_command_list = MakeOwner<D3d12CommandList>(m_graphics_command_list,
                                                 *m_view_cache,
                                                 dynamic_cast<D3d12PipelineStateManager&>(*m_pipeline_state_manager));

    // Create debug buffer.
    {
        size_t bufferSize = sizeof(Vec4f) * 1000;  // hard code
        auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
        D3D_CALL(m_device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_debugVertexData)));
    }
    {
        size_t bufferSize = sizeof(uint32_t) * 3000;  // hard code
        auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
        D3D_CALL(m_device->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_debugIndexData)));
    }

    if (ImGuiService* imgui = m_app->services().imgui) {
        imgui->setRenderCallbacks(
            [this]() {
                ImGui_ImplDX12_Init(m_device.Get(),
                                    kNumFramesInFlight,
                                    d3d::Convert(kDefaultSurfaceFormat),
                                    m_srv_heap.GetHeap(),
                                    m_srv_heap.GetStartCpu(),
                                    m_srv_heap.GetStartGpu());
                ImGui_ImplDX12_NewFrame();
            },
            []() {
                ImGui_ImplDX12_Shutdown();
            });
    }

    return Result<void>();
}

void D3d12RenderDevice::FinalizeImpl() {
    if (m_initialized) {
        cleanupRenderTarget();

        finalizeGraphicsContext();
        m_copyContext.Finalize();
    }
}

void D3d12RenderDevice::render() {
    ID3D12GraphicsCommandList* cmd_list = m_graphics_command_list.Get();

    Vec2i dim = DisplayService::singleton().windowSize();
    CD3DX12_VIEWPORT viewport(0.0f, 0.0f, (float)dim.x, (float)dim.y);
    cmd_list->RSSetViewports(1, &viewport);
    D3D12_RECT rect{ 0, 0, dim.x, dim.y };
    cmd_list->RSSetScissorRects(1, &rect);

    // bind the frame buffer
    D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle = m_depthStencilDescriptor;
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = m_renderTargetDescriptor[m_back_buffer_index];

    // transfer resource state
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_back_buffer_index],
                                                        D3D12_RESOURCE_STATE_PRESENT,
                                                        D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmd_list->ResourceBarrier(1, &barrier);

    cmd_list->OMSetRenderTargets(1, &rtv_handle, FALSE, &dsv_handle);
    float clear_color[4]{ 0, 0, 0, 1 };
    cmd_list->ClearRenderTargetView(rtv_handle, clear_color, 0, nullptr);
    cmd_list->ClearDepthStencilView(dsv_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // @TODO: refactor this
    if (m_app->isRuntime()) {
        CRASH_NOW();
    }

    if (m_app->specification().enableImgui) {
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmd_list);
    }

    barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_back_buffer_index],
                                                   D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                   D3D12_RESOURCE_STATE_PRESENT);
    cmd_list->ResourceBarrier(1, &barrier);
}

void D3d12RenderDevice::present() {
    if (m_app->specification().enableImgui) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }
    D3D_CALL(m_swap_chain->Present(1, 0));  // Present with vsync
}

void D3d12RenderDevice::beginFrame() {
    D3d12FrameContext& frame = reinterpret_cast<D3d12FrameContext&>(getCurrentFrame());
    frame.Wait(m_graphicsFenceEvent, m_graphics_queue_fence.Get());
    D3D_CALL(frame.m_commandAllocator->Reset());
    D3D_CALL(m_graphics_command_list->Reset(frame.m_commandAllocator, nullptr));
    WaitForSingleObject(m_swapChainWaitObject, INFINITE);

    m_back_buffer_index = m_swap_chain->GetCurrentBackBufferIndex();

    ID3D12DescriptorHeap* heap = m_srv_heap.GetHeap();
    m_graphics_command_list->SetDescriptorHeaps(1, &heap);

    m_graphics_command_list->SetGraphicsRootSignature(m_root_signature.Get());
    m_graphics_command_list->SetComputeRootSignature(m_root_signature.Get());

    // Root Param 8: Fixed SRVs (space0)
    CD3DX12_GPU_DESCRIPTOR_HANDLE fixed_srv_handle{ m_srv_heap.GetStartGpu() };
    m_graphics_command_list->SetGraphicsRootDescriptorTable(8, fixed_srv_handle);
}

void D3d12RenderDevice::endFrame() {
    D3D_CALL(m_graphics_command_list->Close());
    ID3D12CommandList* cmdLists[] = { m_graphics_command_list.Get() };
    m_graphics_command_queue->ExecuteCommandLists(std::size(cmdLists), cmdLists);
}

void D3d12RenderDevice::moveToNextFrame() {
    uint64_t fenceValue = m_lastSignaledFenceValue + 1;
    m_graphics_command_queue->Signal(m_graphics_queue_fence.Get(), fenceValue);
    m_lastSignaledFenceValue = fenceValue;

    D3d12FrameContext& frame = reinterpret_cast<D3d12FrameContext&>(getCurrentFrame());
    frame.m_fenceValue = fenceValue;
    m_frame_index = (m_frame_index + 1) % static_cast<uint32_t>(m_frame_contexts.size());
}

Ref<FrameContext> D3d12RenderDevice::createFrameContext() {
    return MakeOwner<D3d12FrameContext>();
}

ID3D12Resource* D3d12RenderDevice::uploadBuffer(uint32_t p_byte_size, const void* p_init_data, ID3D12Resource* p_out_buffer) {
    if (p_byte_size == 0) {
        DEV_ASSERT(p_init_data == nullptr);
        return nullptr;
    }

    if (p_out_buffer == nullptr) {
        auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(p_byte_size);
        // Create the actual default buffer resource.
        D3D_FAIL_V(m_device->CreateCommittedResource(
                       &heap_properties,
                       D3D12_HEAP_FLAG_NONE,
                       &buffer_desc,
                       D3D12_RESOURCE_STATE_COMMON,
                       nullptr,
                       IID_PPV_ARGS(&p_out_buffer)),
                   nullptr);
    }

    // Describe the data we want to copy into the default buffer.
    D3D12_SUBRESOURCE_DATA sub_resource_data = {};
    sub_resource_data.pData = p_init_data;
    sub_resource_data.RowPitch = p_byte_size;
    sub_resource_data.SlicePitch = sub_resource_data.RowPitch;

    auto cmd = m_copyContext.Allocate(p_byte_size);
    UpdateSubresources<1>(cmd.commandList.Get(), p_out_buffer, cmd.uploadBuffer.buffer.Get(), 0, 0, 1, &sub_resource_data);
    m_copyContext.submit(cmd);
    return p_out_buffer;
};

auto D3d12RenderDevice::createBuffer(const GpuBufferDesc& p_desc) -> Result<Ref<GpuBuffer>> {
    auto ret = MakeRef<D3d12Buffer>(p_desc);

    const uint32_t size_in_byte = p_desc.element_count * p_desc.element_size;
    ret->buffer = uploadBuffer(size_in_byte, p_desc.initial_data, nullptr);
    if (ret->buffer == nullptr) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE);
    }

    return ret;
}

auto D3d12RenderDevice::createMeshImpl(const GpuMeshDesc& p_desc,
                                       std::span<const GpuBufferDesc> p_vb_descs,
                                       const GpuBufferDesc* p_ib_desc) -> Result<Ref<GpuMesh>> {
    auto ret = MakeRef<D3d12MeshBuffers>(p_desc);
    for (uint32_t index = 0; index < (uint32_t)p_vb_descs.size(); ++index) {
        const auto& vb_desc = p_vb_descs[index];
        if (vb_desc.element_count == 0) {
            ret->vbvs[index] = { 0, 0, 0 };
            continue;
        }

        auto res = createBuffer(p_vb_descs[index]);
        if (!res) {
            return CAVE_ERROR(res.error());
        }

        ret->vertexBuffers[index] = *res;
        ret->vbvs[index] = {
            .BufferLocation = ((ID3D12Resource*)(ret->vertexBuffers[index]->getHandle()))->GetGPUVirtualAddress(),
            .SizeInBytes = vb_desc.element_count * vb_desc.element_size,
            .StrideInBytes = vb_desc.element_size,
        };
    }

    if (p_ib_desc) {
        auto res = createBuffer(*p_ib_desc);
        if (!res) {
            return CAVE_ERROR(res.error());
        }
        ret->indexBuffer = *res;
        ret->ibv = {
            .BufferLocation = ((ID3D12Resource*)(ret->indexBuffer->getHandle()))->GetGPUVirtualAddress(),
            .SizeInBytes = p_ib_desc->element_count * p_ib_desc->element_size,
            .Format = DXGI_FORMAT_R32_UINT,
        };
    }

    return ret;
}

void D3d12RenderDevice::updateBuffer(const GpuBufferDesc& desc, GpuBuffer* buffer) {
    DEV_ASSERT(desc.element_size == buffer->desc.element_size);
    if (DEV_VERIFY(buffer->desc.element_count >= desc.element_count)) {
        auto d3d_buffer = reinterpret_cast<D3d12Buffer*>(buffer);
        const uint32_t size_in_byte = desc.element_count * desc.element_size;
        uploadBuffer(size_in_byte, desc.initial_data, d3d_buffer->buffer.Get());
    }
}

auto D3d12RenderDevice::createConstantBuffer(const GpuBufferDesc& desc) -> Result<Ref<GpuBuffer>> {
    const uint32_t size_in_byte = desc.element_count * desc.element_size;
    CD3DX12_HEAP_PROPERTIES heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(size_in_byte);
    ComPtr<ID3D12Resource> buffer;
    D3D_FAIL(m_device->CreateCommittedResource(
                 &heap_properties,
                 D3D12_HEAP_FLAG_NONE,
                 &buffer_desc,
                 D3D12_RESOURCE_STATE_GENERIC_READ,
                 nullptr, IID_PPV_ARGS(&buffer)),
             "Failed to create CommittedResource");

    auto result = MakeRef<D3d12Buffer>(desc);
    result->buffer = buffer;

    D3D_FAIL(result->buffer->Map(0, nullptr, reinterpret_cast<void**>(&result->mappedData)),
             "Failed to Map buffer");

    return result;
}

auto D3d12RenderDevice::createStructuredBuffer(const GpuBufferDesc& p_desc) -> Result<Ref<GpuBuffer>> {
    DEV_ASSERT(!p_desc.initial_data && "TODO: initial data");

    CD3DX12_HEAP_PROPERTIES heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    D3D12_RESOURCE_DESC buffer_desc{};
    buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    buffer_desc.Width = p_desc.element_count * p_desc.element_size;
    buffer_desc.Height = 1;
    buffer_desc.DepthOrArraySize = 1;
    buffer_desc.MipLevels = 1;
    buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
    buffer_desc.SampleDesc.Count = 1;
    buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    buffer_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    ComPtr<ID3D12Resource> buffer;
    D3D_FAIL(m_device->CreateCommittedResource(
                 &heap_properties,
                 D3D12_HEAP_FLAG_NONE,
                 &buffer_desc,
                 D3D12_RESOURCE_STATE_COMMON,
                 nullptr, IID_PPV_ARGS(&buffer)),
             "Failed to create buffer (StructuredBuffer)");

    // @TODO: set debug name
    LOG_WARN("TODO: set buffer name");
    // SetDebugName(buffer.Get(), "");
    D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
    uav_desc.Format = DXGI_FORMAT_UNKNOWN;  // Structured buffer doesn't have a format
    uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uav_desc.Buffer.FirstElement = 0;
    uav_desc.Buffer.NumElements = p_desc.element_count;
    uav_desc.Buffer.StructureByteStride = p_desc.element_size;

    auto handle = m_srv_heap.AllocHandle();
    m_device->CreateUnorderedAccessView(buffer.Get(), nullptr, &uav_desc, handle.cpu_handle);

    auto result = MakeRef<D3d12Buffer>(p_desc);
    result->buffer = buffer;
    result->handle = handle;
    return result;
}

void D3d12RenderDevice::updateConstantBuffer(const GpuBuffer* buffer, const void* data, size_t size) {
    if (size) {
        auto d3d_buffer = reinterpret_cast<const D3d12Buffer*>(buffer);
        memcpy(d3d_buffer->mappedData, data, size);
    }
}

Ref<GpuTexture> D3d12RenderDevice::createTextureImpl(const GpuTextureDesc& texture_desc, const SamplerDesc&) {
    auto initial_data = reinterpret_cast<const uint8_t*>(texture_desc.initialData);

    bool gen_mip_map = false;
    PixelFormat format = texture_desc.format;
    DXGI_FORMAT texture_format = d3d::Convert(format);
    DXGI_FORMAT srv_format = d3d::Convert(format);

    D3D12_RESOURCE_STATES initial_state = D3D12_RESOURCE_STATE_COMMON;

    // @TODO: refactor
    switch (format) {
        case PixelFormat::D32_FLOAT: {
            texture_format = DXGI_FORMAT_R32_TYPELESS;
            srv_format = DXGI_FORMAT_R32_FLOAT;
        } break;
        case PixelFormat::D24_UNORM_S8_UINT: {
            texture_format = DXGI_FORMAT_R24G8_TYPELESS;
        } break;
        case PixelFormat::R24G8_TYPELESS: {
            srv_format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        } break;
        case PixelFormat::R32G8X24_TYPELESS: {
            srv_format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        } break;
        default:
            break;
    }
    if (texture_desc.type == AttachmentType::NONE)
        initial_state = D3D12_RESOURCE_STATE_COPY_DEST;
    else if (texture_desc.initialData) {
        initial_state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    }

    D3D12_HEAP_PROPERTIES props{};
    props.Type = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    auto ConvertBindFlags = [](BindFlags p_bind_flags) {
        [[maybe_unused]] constexpr BindFlags supported_flags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET | BIND_DEPTH_STENCIL | BIND_UNORDERED_ACCESS;
        DEV_ASSERT((p_bind_flags & (~supported_flags)) == 0);

        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
        if (p_bind_flags & BIND_RENDER_TARGET) {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        }
        if (p_bind_flags & BIND_DEPTH_STENCIL) {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        }
        if (p_bind_flags & BIND_UNORDERED_ACCESS) {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }

        return flags;
    };

    D3D12_RESOURCE_DESC resource_desc{};
    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resource_desc.Alignment = 0;
    resource_desc.Width = texture_desc.width;
    resource_desc.Height = texture_desc.height;
    resource_desc.DepthOrArraySize = static_cast<UINT16>(texture_desc.arraySize);
    resource_desc.MipLevels = static_cast<UINT16>(gen_mip_map ? 0 : texture_desc.mipLevels);
    resource_desc.Format = texture_format;
    resource_desc.SampleDesc = { 1, 0 };
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resource_desc.Flags = ConvertBindFlags(texture_desc.bindFlags);

    ID3D12Resource* texture_ptr = nullptr;
    D3D_FAIL_V(m_device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &resource_desc, initial_state, NULL, IID_PPV_ARGS(&texture_ptr)), nullptr);

    if (initial_data) {
        // Create a temporary upload resource to move the data in
        uint32_t byte_width = 1;
        switch (texture_desc.format) {
            case PixelFormat::R32G32_FLOAT:
                byte_width = 2;
                break;
            case PixelFormat::R32G32B32_FLOAT:
            case PixelFormat::R32G32B32A32_FLOAT:
                byte_width = 4;
                break;
            default:
                break;
        }
        const uint32_t upload_pitch = byte_width * Align(4 * static_cast<int>(texture_desc.width), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
        const uint32_t upload_size = texture_desc.height * upload_pitch;
        resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resource_desc.Alignment = 0;
        resource_desc.Width = upload_size;
        resource_desc.Height = 1;
        resource_desc.DepthOrArraySize = 1;
        resource_desc.MipLevels = 1;
        resource_desc.Format = DXGI_FORMAT_UNKNOWN;
        resource_desc.SampleDesc = { 1, 0 };
        resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        props.Type = D3D12_HEAP_TYPE_UPLOAD;
        props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

        // @TODO: refactor
        ComPtr<ID3D12Resource> upload_buffer;
        D3D_FAIL_V(m_device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&upload_buffer)), nullptr);

        // Write pixels into the upload resource
        void* mapped = NULL;
        D3D12_RANGE range = { 0, upload_size };

        upload_buffer->Map(0, &range, &mapped);
        const uint32_t byte_per_row = 4 * byte_width * texture_desc.width;
        for (uint32_t y = 0; y < texture_desc.height; y++) {
            memcpy((void*)((uintptr_t)mapped + y * upload_pitch), initial_data + y * byte_per_row, byte_per_row);
        }
        upload_buffer->Unmap(0, &range);

        // Copy the upload resource content into the real resource
        D3D12_TEXTURE_COPY_LOCATION source_location = {};
        source_location.pResource = upload_buffer.Get();
        source_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        source_location.PlacedFootprint.Footprint.Format = d3d::Convert(texture_desc.format);
        source_location.PlacedFootprint.Footprint.Width = texture_desc.width;
        source_location.PlacedFootprint.Footprint.Height = texture_desc.height;
        source_location.PlacedFootprint.Footprint.Depth = 1;
        source_location.PlacedFootprint.Footprint.RowPitch = upload_pitch;

        D3D12_TEXTURE_COPY_LOCATION dest_location = {};
        dest_location.pResource = texture_ptr;
        dest_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dest_location.SubresourceIndex = 0;

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = texture_ptr;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

        // Create a temporary command queue to do the copy with
        ComPtr<ID3D12Fence> fence;
        D3D_FAIL_V(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), nullptr);

        HANDLE event = CreateEvent(0, 0, 0, 0);
        if (event == NULL) {
            return nullptr;
        }

        D3D12_COMMAND_QUEUE_DESC queue_desc = {};
        queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queue_desc.NodeMask = 1;

        ComPtr<ID3D12CommandQueue> copy_queue;
        D3D_FAIL_V(m_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&copy_queue)), nullptr);

        ComPtr<ID3D12CommandAllocator> copy_alloc;
        D3D_FAIL_V(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&copy_alloc)), nullptr);

        ComPtr<ID3D12GraphicsCommandList> command_list;
        D3D_FAIL_V(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, copy_alloc.Get(), NULL, IID_PPV_ARGS(&command_list)), nullptr);

        command_list->CopyTextureRegion(&dest_location, 0, 0, 0, &source_location, NULL);
        command_list->ResourceBarrier(1, &barrier);
        command_list->Close();

        // Execute the copy
        ID3D12CommandList* command_lists[] = { command_list.Get() };
        copy_queue->ExecuteCommandLists(std::size(command_lists), command_lists);
        copy_queue->Signal(fence.Get(), 1);

        // Wait for everything to complete
        fence->SetEventOnCompletion(1, event);
        WaitForSingleObject(event, INFINITE);

        // Tear down our temporary command queue and release the upload resource
        CloseHandle(event);
    }

    auto gpu_texture = MakeRef<D3d12GpuTexture>(texture_desc);
    // Create a shader resource view for the texture
    if (texture_desc.bindFlags & BIND_SHADER_RESOURCE) {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
        srv_desc.Format = srv_format;
        switch (texture_desc.dimension) {
            case Dimension::Texture2D:
                srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srv_desc.Texture2D.MipLevels = resource_desc.MipLevels;
                srv_desc.Texture2D.MostDetailedMip = 0;
                break;
            case Dimension::TextureCube:
                srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                srv_desc.TextureCube.MipLevels = resource_desc.MipLevels;
                srv_desc.TextureCube.MostDetailedMip = 0;
                break;
            case Dimension::TextureCubeArray:
                srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                srv_desc.TextureCubeArray.MipLevels = resource_desc.MipLevels;
                srv_desc.TextureCubeArray.MostDetailedMip = 0;
                srv_desc.TextureCubeArray.First2DArrayFace = 0;
                srv_desc.TextureCubeArray.NumCubes = texture_desc.arraySize / 6;
                break;
            default:
                CRASH_NOW();
                break;
        }

        auto srv_handle = m_srv_heap.AllocHandle();
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        m_device->CreateShaderResourceView(texture_ptr, &srv_desc, srv_handle.cpu_handle);
        gpu_texture->srv_handle = srv_handle;
    }

    if (texture_desc.bindFlags & BIND_UNORDERED_ACCESS) {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.Format = d3d::Convert(texture_desc.format);

        switch (texture_desc.dimension) {
            case Dimension::Texture2D:
                uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                uav_desc.Texture2D.MipSlice = 0;
                uav_desc.Texture2D.PlaneSlice = 0;
                break;
            default:
                CRASH_NOW();
                break;
        }

        auto uav_handle = m_srv_heap.AllocHandle();
        m_device->CreateUnorderedAccessView(texture_ptr, nullptr, &uav_desc, uav_handle.cpu_handle);
        gpu_texture->uav_handle = uav_handle;
    }

    if (initial_data) {
        gpu_texture->current_state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    } else {
        gpu_texture->current_state = initial_state;
    }

    gpu_texture->texture = ComPtr<ID3D12Resource>(texture_ptr);
    SetDebugName(texture_ptr, texture_desc.name);
    return gpu_texture;
}

void D3d12RenderDevice::generateMipmap(const GpuTexture* texture) {
    unused(texture);
}

auto D3d12RenderDevice::createDevice() -> Result<void> {
#if USING(DEBUG_BUILD)
    if (m_enable_validation_layer) {
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug_controller)))) {
            m_debug_controller->EnableDebugLayer();
        }
    }
#endif

    D3D_FAIL(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory)), "failed to create factory");

    auto get_hardware_adapter = [](IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter) {
        *ppAdapter = nullptr;
        for (UINT adapter_index = 0;; ++adapter_index) {
            IDXGIAdapter1* adapter = nullptr;
            if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapter_index, &adapter)) {
                // No more adapters to enumerate.
                break;
            }

            // Check to see if the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr))) {
                *ppAdapter = adapter;
                return;
            }
            adapter->Release();
        }
    };

    ComPtr<IDXGIAdapter1> hardware_adapter;
    get_hardware_adapter(m_factory.Get(), &hardware_adapter);

    if (FAILED(D3D12CreateDevice(hardware_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(m_device.GetAddressOf())))) {
        ComPtr<IDXGIAdapter> warp_adapter;
        D3D_FAIL(m_factory->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter)), "failed to enum warp adapter");

        D3D_FAIL(D3D12CreateDevice(warp_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(m_device.GetAddressOf())),
                 "failed to create d3d device");
    }

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0 };

    D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels = { std::size(featureLevels), featureLevels, D3D_FEATURE_LEVEL_12_0 };

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_0;
    D3D_CALL(m_device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels)));
    featureLevel = featLevels.MaxSupportedFeatureLevel;
    switch (featureLevel) {
        case D3D_FEATURE_LEVEL_12_0:
            LOG_INFO("[DX12] Device Feature Level: 12.0");
            break;
        case D3D_FEATURE_LEVEL_12_1:
            LOG_INFO("[DX12] Device Feature Level: 12.1");
            break;
        default:
            CRASH_NOW();
            break;
    }

    return Result<void>();
}

auto D3d12RenderDevice::initGraphicsContext() -> Result<void> {
    m_graphics_command_queue = createCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    DEV_ASSERT(m_graphics_command_queue);

    [[maybe_unused]] int frame_idx = 0;
    for (auto& it : m_frame_contexts) {
        D3d12FrameContext& frame = reinterpret_cast<D3d12FrameContext&>(*it.get());
        D3D_FAIL(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame.m_commandAllocator)),
                 "failed to create command allocator");
        D3D12_SET_DEBUG_NAME(frame.m_commandAllocator, std::format("GraphicsCommandAllocator {}", frame_idx++));

        if (m_graphics_command_list == nullptr) {
            D3D_FAIL(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frame.m_commandAllocator, nullptr, IID_PPV_ARGS(&m_graphics_command_list)),
                     "failed to create graphics command list");
        }
    }

    m_graphics_command_list->Close();
    D3D12_SET_DEBUG_NAME(m_graphics_command_list.Get(), "GraphicsCommandList");

    D3D_FAIL(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_graphics_queue_fence)),
             "failed to create fence");

    D3D12_SET_DEBUG_NAME(m_graphics_queue_fence.Get(), "GraphicsFence");

    m_graphicsFenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

    return Result<void>();
}

void D3d12RenderDevice::finalizeGraphicsContext() {
    flushGraphicsContext();

    m_graphics_queue_fence.Reset();
    m_lastSignaledFenceValue = 0;

    CloseHandle(m_graphicsFenceEvent);
    m_graphicsFenceEvent = NULL;

    m_graphics_command_list.Reset();
    m_graphics_command_queue.Reset();

    m_frame_contexts.clear();
}

void D3d12RenderDevice::flushGraphicsContext() {
    for (auto& it : m_frame_contexts) {
        D3d12FrameContext& frame = reinterpret_cast<D3d12FrameContext&>(*it.get());
        frame.Wait(m_graphicsFenceEvent, m_graphics_queue_fence.Get());
    }
    m_frame_index = 0;
}

ID3D12CommandQueue* D3d12RenderDevice::createCommandQueue(D3D12_COMMAND_LIST_TYPE p_type) {
    D3D12_COMMAND_QUEUE_DESC desc;
    desc.Type = p_type;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

    ID3D12CommandQueue* queue;

    D3D_CALL(m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&queue)));

#if USING(USE_D3D_DEBUG_NAME)
    switch (p_type) {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            D3D12_SET_DEBUG_NAME(queue, "GraphicsCommandQueue");
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            D3D12_SET_DEBUG_NAME(queue, "ComputeCommandQueue");
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            D3D12_SET_DEBUG_NAME(queue, "CopyCommandQueue");
            break;
        default:
            CRASH_NOW();
            break;
    }
#endif

    return queue;
}

auto D3d12RenderDevice::enableDebugLayer() -> Result<void> {
#if USING(DEBUG_BUILD)
    D3D_FAIL(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug_controller)),
             "failed to get debug interface");

    m_debug_controller->EnableDebugLayer();

    ComPtr<ID3D12Debug> debug_device;
    m_device->QueryInterface(IID_PPV_ARGS(debug_device.GetAddressOf()));

    ComPtr<ID3D12InfoQueue> info_queue;

    D3D_FAIL(m_device->QueryInterface(IID_PPV_ARGS(&info_queue)), "failed to query info queue interface");

    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);

    D3D12_MESSAGE_ID ignore_list[] = {
        D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
        D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
        D3D12_MESSAGE_ID_COMMAND_LIST_DRAW_VERTEX_BUFFER_NOT_SET,
    };

    D3D12_INFO_QUEUE_FILTER filter = {};
    filter.DenyList.pIDList = ignore_list;
    filter.DenyList.NumIDs = std::size(ignore_list);
    info_queue->AddRetrievalFilterEntries(&filter);
    info_queue->AddStorageFilterEntries(&filter);
#endif
    return Result<void>();
}

auto D3d12RenderDevice::createDescriptorHeaps() -> Result<void> {
    if (auto res = m_rtv_heap.Initialize(128, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_device.Get(), false); !res) {
        return CAVE_ERROR(res.error());
    }
    if (auto res = m_dsv_heap.Initialize(128, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_device.Get(), false); !res) {
        return CAVE_ERROR(res.error());
    }
    if (auto res = m_srv_heap.Initialize(512, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_device.Get(), true); !res) {
        return CAVE_ERROR(res.error());
    }

    return Result<void>();
}

auto D3d12RenderDevice::createSwapChain(uint32_t p_width, uint32_t p_height) -> Result<void> {
    auto display_manager = dynamic_cast<GlfwDisplayService*>(DisplayService::singletonPtr());
    DEV_ASSERT(display_manager);

    // create a struct to hold information about the swap chain
    DXGI_SWAP_CHAIN_DESC1 scd{};

    // fill the swap chain description struct
    scd.Width = p_width;
    scd.Height = p_height;
    scd.Format = d3d::Convert(kDefaultSurfaceFormat);
    scd.Stereo = FALSE;
    scd.SampleDesc = { 1, 0 };
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // how swap chain is to be used
    scd.BufferCount = kNumFramesInFlight;               // back buffer count
    scd.Scaling = DXGI_SCALING_STRETCH;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

    IDXGISwapChain1* pSwapChain = nullptr;

    D3D_FAIL(m_factory->CreateSwapChainForHwnd(
                 m_graphics_command_queue.Get(),
                 static_cast<HWND>(display_manager->nativeWindow()),
                 &scd,
                 NULL,
                 NULL,
                 &pSwapChain),
             "Failed to create swapchain");

    m_swap_chain.Attach(reinterpret_cast<IDXGISwapChain3*>(pSwapChain));

    m_swap_chain->SetMaximumFrameLatency(kNumBackBuffers);
    m_swapChainWaitObject = m_swap_chain->GetFrameLatencyWaitableObject();

    return Result<void>();
}

auto D3d12RenderDevice::createRenderTarget(uint32_t p_width, uint32_t p_height) -> Result<void> {
    for (int32_t i = 0; i < kNumFramesInFlight; i++) {
        ID3D12Resource* backbuffer = nullptr;
        D3D_CALL(m_swap_chain->GetBuffer(i, IID_PPV_ARGS(&backbuffer)));
        m_device->CreateRenderTargetView(backbuffer, nullptr, m_renderTargetDescriptor[i]);
        std::wstring name = std::wstring(L"Render Target Buffer") + std::to_wstring(i);
        backbuffer->SetName(name.c_str());
        m_renderTargets[i] = backbuffer;
    }

    D3D12_CLEAR_VALUE depthOptimizedClearValue{};
    depthOptimizedClearValue.Format = d3d::Convert(kDefaultDepthStencilFormat);
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES prop{};
    prop.Type = D3D12_HEAP_TYPE_DEFAULT;
    prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    prop.CreationNodeMask = 1;
    prop.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resource_desc{};
    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resource_desc.Alignment = 0;
    resource_desc.Width = p_width;
    resource_desc.Height = p_height;
    resource_desc.DepthOrArraySize = 1;
    resource_desc.MipLevels = 1;
    resource_desc.Format = d3d::Convert(kDefaultDepthStencilFormat);
    resource_desc.SampleDesc = { 1, 0 };
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resource_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    HRESULT hr = m_device->CreateCommittedResource(
        &prop, D3D12_HEAP_FLAG_NONE, &resource_desc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthOptimizedClearValue,
        IID_PPV_ARGS(&m_depthStencilBuffer));

    D3D_FAIL(hr, "Failed to create committed resource");

    m_depthStencilBuffer->SetName(L"Depth Stencil Buffer");
    m_device->CreateDepthStencilView(m_depthStencilBuffer, nullptr, m_depthStencilDescriptor);

    return Result<void>();
}

void D3d12RenderDevice::initStaticSamplers() {
    auto FillSamplerDesc = [](uint32_t p_slot, const SamplerDesc& p_desc) {
        CD3DX12_STATIC_SAMPLER_DESC sampler_desc(
            p_slot,
            d3d::Convert(p_desc.min_filter, p_desc.mag_filter),
            d3d::Convert(p_desc.address_u),
            d3d::Convert(p_desc.address_v),
            d3d::Convert(p_desc.address_w),
            p_desc.mip_lod_bias,
            p_desc.max_anisotropy,
            d3d::Convert(p_desc.comparison_func),
            d3d::Convert(p_desc.static_border_color),
            p_desc.min_lod,
            p_desc.max_lod);

        return sampler_desc;
    };

#define SAMPLER_STATE(REG, NAME, DESC) m_staticSamplers.emplace_back(FillSamplerDesc(REG, DESC));
#include "sampler.slang.h"
#undef SAMPLER_STATE
}

auto D3d12RenderDevice::createRootSignature() -> Result<void> {
    CD3DX12_ROOT_PARAMETER params[13]{};
    int idx = 0;

    // Root Params 0..7: Inline CBVs (b0..b7 space0)
    for (UINT i = 0; i < 8; ++i) {
        params[idx++].InitAsConstantBufferView(i);
    }

    // Descriptor Ranges
    CD3DX12_DESCRIPTOR_RANGE ranges[2]{};
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 32, 0, 0);  // Param 8:  t0..t31 space0 (Fixed SRVs)
    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 16, 0, 0);  // Param 9:  u0..u15 space0 (Fixed UAVs / Compute)

    // Assign Descriptor Tables (Params 8...9)
    for (size_t i = 0; i < std::size(ranges); ++i) {
        params[idx++].InitAsDescriptorTable(1, &ranges[i]);
    }

    initStaticSamplers();

    const D3D12_ROOT_SIGNATURE_FLAGS root_sig_flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

    CD3DX12_ROOT_SIGNATURE_DESC root_sig_desc;
    root_sig_desc.Init(idx,
                       params,
                       static_cast<uint32_t>(m_staticSamplers.size()),
                       m_staticSamplers.data(),
                       root_sig_flags);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    HRESULT hr = D3D12SerializeRootSignature(&root_sig_desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &error);
    if (FAILED(hr)) {
        const char* errorMsg = error ? static_cast<const char*>(error->GetBufferPointer()) : "Unknown error";
        LOG_ERROR("Failed to serialize root signature: {}", errorMsg);
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Failed to serialize root signature");
    }

    D3D_FAIL(m_device->CreateRootSignature(0,
                                           signature->GetBufferPointer(),
                                           signature->GetBufferSize(),
                                           IID_PPV_ARGS(&m_root_signature)),
             "Failed to create root signature");

    return Result<void>();
}

void D3d12RenderDevice::onWindowResize(int width, int height) {
    if (width <= 0 || height <= 0) return;
    if (m_swap_chain) {
        cleanupRenderTarget();
        D3D_CALL(m_swap_chain->ResizeBuffers(0, width, height,
                                             DXGI_FORMAT_UNKNOWN,
                                             DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT));

        [[maybe_unused]] auto err = createRenderTarget(width, height);
    }
}

void D3d12RenderDevice::cleanupRenderTarget() {
    flushGraphicsContext();

    for (uint32_t i = 0; i < kNumBackBuffers; ++i) {
        SafeRelease(m_renderTargets[i]);
    }
    SafeRelease(m_depthStencilBuffer);
}

ICommandList* D3d12RenderDevice::getCommandList() const {
    return m_command_list.get();
}

}  // namespace cave::render
