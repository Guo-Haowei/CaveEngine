#include "D3d12RenderDevice.h"

#include <imgui/backends/imgui_impl_dx12.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "D3d12PipelineStateManager.h"
#include "D3d12Resources.h"
#include "D3d12ViewCache.h"

#include "cave/core/string/StringUtils.h"
#include "cave/runtime/framework/IApplication.h"

#include "../d3d_common/D3dCommon.h"
#include "engine/private/core/math/MatrixTransform.h"
#include "engine/private/renderer/graphics_private.h"
#include "engine/private/renderer/sampler.h"
#include "engine/private/runtime/display/GlfwDisplayService.h"
#include "engine/private/runtime/framework/ImGuiManager.h"
#include "engine/private/runtime/scene/Scene.h"

#define INCLUDE_AS_D3D12
#include "../d3d_common/D3dConvert.h"

namespace cave {
#include "structured_buffer.hlsl.h"
}  // namespace cave

namespace cave::render {

using namespace cave::math;
using Microsoft::WRL::ComPtr;

struct D3d12ConstantBuffer : public GpuConstantBuffer {
    using GpuConstantBuffer::GpuConstantBuffer;

    ~D3d12ConstantBuffer() {
        if (buffer) {
            buffer->Unmap(0, nullptr);
        }
        mappedData = nullptr;
    }

    Microsoft::WRL::ComPtr<ID3D12Resource> buffer{};
    uint8_t* mappedData{ nullptr };
};

struct D3d12StructuredBuffer : public GpuStructuredBuffer {
    using GpuStructuredBuffer::GpuStructuredBuffer;

    Microsoft::WRL::ComPtr<ID3D12Resource> buffer{};
    DescriptorHeapHandle handle;
};

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
    : RenderDevice("D3d12RenderDevice", rhi::Backend::D3d12, NUM_FRAMES_IN_FLIGHT) {
    m_pipeline_state_manager = MakeOwner<D3d12PipelineStateManager>(this);
}

auto D3d12RenderDevice::InitializeInternal() -> Result<void> {
    const int w = DisplayService::singleton().windowSize().x;
    const int h = DisplayService::singleton().windowSize().y;
    DEV_ASSERT(w > 0 && h > 0);

    if (auto res = createDevice(); !res) {
        return CAVE_ERROR(res.error());
    }

    if (m_enableValidationLayer) {
        if (enableDebugLayer()) {
            LOG_INFO("[GraphicsManager_DX12] Debug layer enabled");
        } else {
            LOG_ERROR("[GraphicsManager_DX12] Debug layer not enabled");
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

    for (int i = 0; i < NUM_BACK_BUFFERS; i++) {
        auto handle = m_rtvDescHeap.AllocHandle();
        m_renderTargetDescriptor[i] = handle.cpuHandle;
    }

    {
        auto handle = m_dsvDescHeap.AllocHandle();
        m_depthStencilDescriptor = handle.cpuHandle;
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

    m_view_cache = MakeOwner<D3d12ViewCache>(m_device.Get(), m_rtvDescHeap, m_dsvDescHeap);

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
                                    NUM_FRAMES_IN_FLIGHT,
                                    d3d::Convert(DEFAULT_SURFACE_FORMAT),
                                    m_srvDescHeap.GetHeap(),
                                    m_srvDescHeap.GetStartCpu(),
                                    m_srvDescHeap.GetStartGpu());
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
    ID3D12GraphicsCommandList* cmd_list = m_graphicsCommandList.Get();

    Vec2i dim = DisplayService::singleton().windowSize();
    CD3DX12_VIEWPORT viewport(0.0f, 0.0f, (float)dim.x, (float)dim.y);
    cmd_list->RSSetViewports(1, &viewport);
    D3D12_RECT rect{ 0, 0, dim.x, dim.y };
    cmd_list->RSSetScissorRects(1, &rect);

    // bind the frame buffer
    D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle = m_depthStencilDescriptor;
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = m_renderTargetDescriptor[m_backbufferIndex];

    // transfer resource state
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_backbufferIndex],
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
        // RenderGraphBuilder::DrawDebugImages(*GetRenderData(),
        //                                               width,
        //                                               height,
        //                                               *this);
    }

    if (m_app->specification().enableImgui) {
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmd_list);
    }

    barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_backbufferIndex],
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
    D3D_CALL(m_swapChain->Present(1, 0));  // Present with vsync
}

void D3d12RenderDevice::beginFrame() {
    // @TODO: wait for swap chain
    D3d12FrameContext& frame = reinterpret_cast<D3d12FrameContext&>(getCurrentFrame());
    frame.Wait(m_graphicsFenceEvent, m_graphicsQueueFence.Get());
    D3D_CALL(frame.m_commandAllocator->Reset());
    D3D_CALL(m_graphicsCommandList->Reset(frame.m_commandAllocator, nullptr));

    WaitForSingleObject(m_swapChainWaitObject, INFINITE);

    m_backbufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    m_graphicsCommandList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_graphicsCommandList->SetComputeRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* heap = m_srvDescHeap.GetHeap();
    m_graphicsCommandList->SetDescriptorHeaps(1, &heap);

    // @TODO: NO HARDCODE
    CD3DX12_GPU_DESCRIPTOR_HANDLE handle{ m_srvDescHeap.GetStartGpu() };
    m_graphicsCommandList->SetGraphicsRootDescriptorTable(7, handle);
    m_graphicsCommandList->SetComputeRootDescriptorTable(7, handle);
}

void D3d12RenderDevice::endFrame() {
    D3D_CALL(m_graphicsCommandList->Close());
    ID3D12CommandList* cmdLists[] = { m_graphicsCommandList.Get() };
    m_graphicsCommandQueue->ExecuteCommandLists(std::size(cmdLists), cmdLists);
}

void D3d12RenderDevice::moveToNextFrame() {
    uint64_t fenceValue = m_lastSignaledFenceValue + 1;
    m_graphicsCommandQueue->Signal(m_graphicsQueueFence.Get(), fenceValue);
    m_lastSignaledFenceValue = fenceValue;

    D3d12FrameContext& frame = reinterpret_cast<D3d12FrameContext&>(getCurrentFrame());
    frame.m_fenceValue = fenceValue;
    m_frameIndex = (m_frameIndex + 1) % static_cast<uint32_t>(m_frameContexts.size());
}

Ref<FrameContext> D3d12RenderDevice::createFrameContext() {
    return MakeOwner<D3d12FrameContext>();
}

void D3d12RenderDevice::setStencilRef(uint32_t p_ref) {
    m_graphicsCommandList->OMSetStencilRef(p_ref);
}

void D3d12RenderDevice::setRenderTargets(const RenderTargetDesc& p_desc) {
    constexpr size_t kMaxRenderTargets = 8;
    DEV_ASSERT(p_desc.colors.size() <= kMaxRenderTargets);

    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, kMaxRenderTargets> rtvs{};
    uint32_t rtv_count = 0;

    // 1. Transition and collect color attachment RTVs
    for (const auto& color : p_desc.colors) {
        if (!color.tex) continue;

        auto* d3d_tex = static_cast<D3d12GpuTexture*>(color.tex.get());

        // State transition barrier
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            d3d_tex->texture.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_graphicsCommandList->ResourceBarrier(1, &barrier);

        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = m_view_cache->getOrCreateRtv(color);
        rtvs[rtv_count++] = rtv_handle;

        if (color.load == LoadOp::Clear) {
            m_graphicsCommandList->ClearRenderTargetView(rtv_handle, color.clear_color, 0, nullptr);
        }
    }

    // 2. Transition and collect depth attachment DSV
    D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle{};
    const D3D12_CPU_DESCRIPTOR_HANDLE* dsv_ptr = nullptr;

    if (p_desc.depth && p_desc.depth->tex) {
        auto* d3d_depth = static_cast<D3d12GpuTexture*>(p_desc.depth->tex.get());

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            d3d_depth->texture.Get(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_DEPTH_WRITE);
        m_graphicsCommandList->ResourceBarrier(1, &barrier);

        dsv_handle = m_view_cache->getOrCreateDsv(*p_desc.depth);
        dsv_ptr = &dsv_handle;

        UINT clear_flags = 0;
        if (p_desc.depth->depth_load == LoadOp::Clear) clear_flags |= D3D12_CLEAR_FLAG_DEPTH;
        if (p_desc.depth->stencil_load == LoadOp::Clear) clear_flags |= D3D12_CLEAR_FLAG_STENCIL;

        if (clear_flags) {
            m_graphicsCommandList->ClearDepthStencilView(
                dsv_handle,
                static_cast<D3D12_CLEAR_FLAGS>(clear_flags),
                p_desc.depth->clear_depth,
                p_desc.depth->clear_stencil,
                0, nullptr);
        }
    }

    // 3. Bind targets
    m_graphicsCommandList->OMSetRenderTargets(rtv_count, rtvs.data(), FALSE, dsv_ptr);
}

void D3d12RenderDevice::unsetRenderTargets() {
    m_graphicsCommandList->OMSetRenderTargets(0, nullptr, FALSE, nullptr);
}

void D3d12RenderDevice::beginPass(const CompiledPass& p_pass) {
    RenderDevice::beginPass(p_pass);

    ID3D12GraphicsCommandList* command_list = m_graphicsCommandList.Get();
    for (auto& texture : p_pass.srvs) {
        D3D12_RESOURCE_STATES resource_state{};
        if (texture->desc.bindFlags & BIND_RENDER_TARGET) {
            resource_state = D3D12_RESOURCE_STATE_RENDER_TARGET;
        } else if (texture->desc.bindFlags & BIND_DEPTH_STENCIL) {
            resource_state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        } else {
            CRASH_NOW();
        }

        auto d3d_texture = reinterpret_cast<D3d12GpuTexture*>(texture.get());
        auto barriers = CD3DX12_RESOURCE_BARRIER::Transition(d3d_texture->texture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, resource_state);
        command_list->ResourceBarrier(1, &barriers);
    }
}

void D3d12RenderDevice::endPass(const CompiledPass& p_pass) {
    RenderDevice::endPass(p_pass);

    unsetRenderTargets();
    ID3D12GraphicsCommandList* command_list = m_graphicsCommandList.Get();
    for (auto& texture : p_pass.srvs) {
        D3D12_RESOURCE_STATES resource_state{};
        if (texture->desc.bindFlags & BIND_RENDER_TARGET) {
            resource_state = D3D12_RESOURCE_STATE_RENDER_TARGET;
        } else if (texture->desc.bindFlags & BIND_DEPTH_STENCIL) {
            resource_state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        } else {
            CRASH_NOW();
        }

        auto d3d_texture = reinterpret_cast<D3d12GpuTexture*>(texture.get());
        auto barriers = CD3DX12_RESOURCE_BARRIER::Transition(d3d_texture->texture.Get(), resource_state, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        command_list->ResourceBarrier(1, &barriers);
    }
}

void D3d12RenderDevice::clear(const RenderTargetDesc&) {
    DEV_ASSERT(0);
}

void D3d12RenderDevice::setViewport(const Viewport& viewport) {
    D3D12_VIEWPORT vp{
        .TopLeftX = static_cast<float>(viewport.topLeftX),
        .TopLeftY = static_cast<float>(viewport.topLeftY),
        .Width = static_cast<float>(viewport.width),
        .Height = static_cast<float>(viewport.height),
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f,
    };

    D3D12_RECT rect{
        .left = 0,
        .top = 0,
        .right = viewport.topLeftX + viewport.width,
        .bottom = viewport.topLeftY + viewport.height,
    };

    m_graphicsCommandList->RSSetViewports(1, &vp);
    m_graphicsCommandList->RSSetScissorRects(1, &rect);
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

auto D3d12RenderDevice::createBuffer(const GpuBufferDesc& p_desc) -> Result<std::shared_ptr<GpuBuffer>> {
    auto ret = std::make_shared<D3d12Buffer>(p_desc);

    const uint32_t size_in_byte = p_desc.element_count * p_desc.element_size;
    ret->buffer = uploadBuffer(size_in_byte, p_desc.initial_data, nullptr);
    if (ret->buffer == nullptr) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE);
    }

    return ret;
}

auto D3d12RenderDevice::createMeshImpl(const GpuMeshDesc& p_desc,
                                       std::span<const GpuBufferDesc> p_vb_descs,
                                       const GpuBufferDesc* p_ib_desc) -> Result<std::shared_ptr<GpuMesh>> {
    auto ret = std::make_shared<D3d12MeshBuffers>(p_desc);
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
            .BufferLocation = ((ID3D12Resource*)(ret->vertexBuffers[index]->GetHandle()))->GetGPUVirtualAddress(),
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
            .BufferLocation = ((ID3D12Resource*)(ret->indexBuffer->GetHandle()))->GetGPUVirtualAddress(),
            .SizeInBytes = p_ib_desc->element_count * p_ib_desc->element_size,
            .Format = DXGI_FORMAT_R32_UINT,
        };
    }

    return ret;
}

void D3d12RenderDevice::setMesh(const GpuMesh* p_mesh) {
    auto mesh = reinterpret_cast<const D3d12MeshBuffers*>(p_mesh);

    m_graphicsCommandList->IASetVertexBuffers(0, MESH_MAX_VERTEX_BUFFER_COUNT, mesh->vbvs);
    if (mesh->indexBuffer) {
        m_graphicsCommandList->IASetIndexBuffer(&mesh->ibv);
    }
}

void D3d12RenderDevice::updateBuffer(const GpuBufferDesc& p_desc, GpuBuffer* p_buffer) {
    DEV_ASSERT(p_desc.element_size == p_buffer->desc.element_size);
    if (DEV_VERIFY(p_buffer->desc.element_count >= p_desc.element_count)) {
        auto buffer = reinterpret_cast<D3d12Buffer*>(p_buffer);
        const uint32_t size_in_byte = p_desc.element_count * p_desc.element_size;
        uploadBuffer(size_in_byte, p_desc.initial_data, buffer->buffer.Get());
    }
}

void D3d12RenderDevice::drawElements(uint32_t p_count, uint32_t p_offset) {
    m_graphicsCommandList->DrawIndexedInstanced(p_count, 1, p_offset, 0, 0);
}

void D3d12RenderDevice::drawElementsInstanced(uint32_t p_instance_count, uint32_t p_count, uint32_t p_offset) {
    m_graphicsCommandList->DrawIndexedInstanced(p_count, p_instance_count, p_offset, 0, 0);
}

void D3d12RenderDevice::drawArrays(uint32_t p_count, uint32_t p_offset) {
    m_graphicsCommandList->DrawInstanced(p_count, 1, p_offset, 0);
}

void D3d12RenderDevice::drawArraysInstanced(uint32_t p_instance_count, uint32_t p_count, uint32_t p_offset) {
    m_graphicsCommandList->DrawInstanced(p_count, p_instance_count, p_offset, 0);
}

void D3d12RenderDevice::dispatch(uint32_t p_num_groups_x, uint32_t p_num_groups_y, uint32_t p_num_groups_z) {
    m_graphicsCommandList->Dispatch(p_num_groups_x, p_num_groups_y, p_num_groups_z);
}

void D3d12RenderDevice::bindUnorderedAccessView(uint32_t p_slot, GpuTexture* p_texture) {
    unused(p_slot);
    unused(p_texture);
}

void D3d12RenderDevice::unbindUnorderedAccessView(uint32_t p_slot) {
    unused(p_slot);
}

void D3d12RenderDevice::bindStructuredBuffer(int p_slot, const GpuStructuredBuffer* p_buffer) {
    unused(p_slot);
    auto uav = reinterpret_cast<const D3d12StructuredBuffer*>(p_buffer);
    if (DEV_VERIFY(uav)) {
    }
}

void D3d12RenderDevice::unbindStructuredBuffer(int p_slot) {
    unused(p_slot);
}

void D3d12RenderDevice::bindStructuredBufferSRV(int p_slot, const GpuStructuredBuffer* p_buffer) {
    unused(p_slot);
    unused(p_buffer);
}

void D3d12RenderDevice::unbindStructuredBufferSRV(int p_slot) {
    unused(p_slot);
}

auto D3d12RenderDevice::createConstantBuffer(const GpuBufferDesc& p_desc) -> Result<std::shared_ptr<GpuConstantBuffer>> {
    const uint32_t size_in_byte = p_desc.element_count * p_desc.element_size;
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

    auto result = std::make_shared<D3d12ConstantBuffer>(p_desc);
    result->buffer = buffer;

    D3D_FAIL(result->buffer->Map(0, nullptr, reinterpret_cast<void**>(&result->mappedData)),
             "Failed to Map buffer");

    return result;
}

auto D3d12RenderDevice::createStructuredBuffer(const GpuBufferDesc& p_desc) -> Result<std::shared_ptr<GpuStructuredBuffer>> {
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

    auto handle = m_srvDescHeap.AllocHandle();
    m_device->CreateUnorderedAccessView(buffer.Get(), nullptr, &uav_desc, handle.cpuHandle);

    auto result = std::make_shared<D3d12StructuredBuffer>(p_desc);
    result->buffer = buffer;
    result->handle = handle;
    return result;
}

void D3d12RenderDevice::updateConstantBuffer(const GpuConstantBuffer* p_buffer, const void* p_data, size_t p_size) {
    if (p_size) {
        auto cb = reinterpret_cast<const D3d12ConstantBuffer*>(p_buffer);
        memcpy(cb->mappedData, p_data, p_size);
    }
}

void D3d12RenderDevice::bindConstantBufferRange(const GpuConstantBuffer* p_buffer, uint32_t p_size, uint32_t p_offset) {
    auto buffer = reinterpret_cast<const D3d12ConstantBuffer*>(p_buffer);
    DEV_ASSERT(p_size + p_offset <= buffer->capacity);

    D3D12_GPU_VIRTUAL_ADDRESS batch_address = buffer->buffer->GetGPUVirtualAddress();

    batch_address += p_offset;
    m_graphicsCommandList->SetGraphicsRootConstantBufferView(buffer->desc.slot, batch_address);
    m_graphicsCommandList->SetComputeRootConstantBufferView(buffer->desc.slot, batch_address);
}

Ref<GpuTexture> D3d12RenderDevice::createTextureImpl(const GpuTextureDesc& texture_desc, const SamplerDesc&) {
    auto initial_data = reinterpret_cast<const uint8_t*>(texture_desc.initialData);

    bool gen_mip_map = false;
    PixelFormat format = texture_desc.format;
    DXGI_FORMAT texture_format = d3d::Convert(format);
    DXGI_FORMAT srv_format = d3d::Convert(format);
    D3D12_RESOURCE_STATES initial_state = D3D12_RESOURCE_STATE_COPY_DEST;

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
    switch (texture_desc.type) {
        case AttachmentType::NONE:
            initial_state = D3D12_RESOURCE_STATE_COPY_DEST;
            break;
        default:
            initial_state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            break;
    }

    D3D12_HEAP_PROPERTIES props{};
    props.Type = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    auto ConvertBindFlags = [](BindFlags p_bind_flags) {
        [[maybe_unused]] constexpr BindFlags supported_flags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET | BIND_DEPTH_STENCIL | BIND_UNORDERED_ACCESS;
        DEV_ASSERT((p_bind_flags & (~supported_flags)) == 0);

        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
        // if (p_bind_flags & BIND_SHADER_RESOURCE) {
        // }
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

    auto gpu_texture = std::make_shared<D3d12GpuTexture>(texture_desc);
    // Create a shader resource view for the texture
    if (texture_desc.bindFlags & BIND_SHADER_RESOURCE) {
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
        srv_desc.Format = srv_format;
        DescriptorResourceType resource_type{};
        switch (texture_desc.dimension) {
            case Dimension::Texture2D:
                srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srv_desc.Texture2D.MipLevels = resource_desc.MipLevels;
                srv_desc.Texture2D.MostDetailedMip = 0;

                resource_type = DescriptorResourceType::Texture2D;
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

                resource_type = DescriptorResourceType::TextureCubeArray;
                break;
            default:
                CRASH_NOW();
                break;
        }

        auto srv_handle = m_srvDescHeap.AllocBindlessHandle(resource_type);
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        m_device->CreateShaderResourceView(texture_ptr, &srv_desc, srv_handle.cpuHandle);
        gpu_texture->srvHandle = srv_handle;
    }

    if (texture_desc.bindFlags & BIND_UNORDERED_ACCESS) {
        LOG_ERROR("@TODO: fix hard code");
#if 0
        D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
        uav_desc.Format = texture_format;
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uav_desc.Texture2D.MipSlice = 0;
        auto uav_handle = m_srvDescHeap.AllocBindlessHandle(DescriptorResourceType::RWTexture2D);
        m_device->CreateUnorderedAccessView(texture_ptr, nullptr, &uav_desc, uav_handle.cpuHandle);
        gpu_texture->uavHandle = uav_handle;
#endif
    }

    gpu_texture->texture = ComPtr<ID3D12Resource>(texture_ptr);
    SetDebugName(texture_ptr, texture_desc.name);
    return gpu_texture;
}

void D3d12RenderDevice::bindTexture(Dimension, uint64_t, int) {
}

void D3d12RenderDevice::unbindTexture(Dimension, int) {
}

void D3d12RenderDevice::generateMipmap(const GpuTexture* p_texture) {
    unused(p_texture);
    CRASH_NOW();
}

auto D3d12RenderDevice::createDevice() -> Result<void> {
#if USING(DEBUG_BUILD)
    if (m_enableValidationLayer) {
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController)))) {
            m_debugController->EnableDebugLayer();
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
    m_graphicsCommandQueue = createCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    DEV_ASSERT(m_graphicsCommandQueue);

    [[maybe_unused]] int frame_idx = 0;
    for (auto& it : m_frameContexts) {
        D3d12FrameContext& frame = reinterpret_cast<D3d12FrameContext&>(*it.get());
        D3D_FAIL(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame.m_commandAllocator)),
                 "failed to create command allocator");
        D3D12_SET_DEBUG_NAME(frame.m_commandAllocator, std::format("GraphicsCommandAllocator {}", frame_idx++));

        if (m_graphicsCommandList == nullptr) {
            D3D_FAIL(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frame.m_commandAllocator, nullptr, IID_PPV_ARGS(&m_graphicsCommandList)),
                     "failed to create graphics command list");
        }
    }

    m_graphicsCommandList->Close();
    D3D12_SET_DEBUG_NAME(m_graphicsCommandList.Get(), "GraphicsCommandList");

    D3D_FAIL(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_graphicsQueueFence)),
             "failed to create fence");

    D3D12_SET_DEBUG_NAME(m_graphicsQueueFence.Get(), "GraphicsFence");

    m_graphicsFenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

    return Result<void>();
}

void D3d12RenderDevice::finalizeGraphicsContext() {
    flushGraphicsContext();

    m_graphicsQueueFence.Reset();
    m_lastSignaledFenceValue = 0;

    CloseHandle(m_graphicsFenceEvent);
    m_graphicsFenceEvent = NULL;

    m_graphicsCommandList.Reset();
    m_graphicsCommandQueue.Reset();

    m_frameContexts.clear();
}

void D3d12RenderDevice::flushGraphicsContext() {
    for (auto& it : m_frameContexts) {
        D3d12FrameContext& frame = reinterpret_cast<D3d12FrameContext&>(*it.get());
        frame.Wait(m_graphicsFenceEvent, m_graphicsQueueFence.Get());
    }
    m_frameIndex = 0;
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
    D3D_FAIL(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController)),
             "failed to get debug interface");

    m_debugController->EnableDebugLayer();

    ComPtr<ID3D12Debug> debug_device;
    m_device->QueryInterface(IID_PPV_ARGS(debug_device.GetAddressOf()));

    ComPtr<ID3D12InfoQueue> info_queue;

    D3D_FAIL(m_device->QueryInterface(IID_PPV_ARGS(&info_queue)),
             "failed to query info queue interface");

    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
    info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);

    D3D12_MESSAGE_ID ignore_list[] = {
        D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
        D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
        // D3D12_MESSAGE_ID_COPY_DESCRIPTORS_INVALID_RANGES
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
    if (auto res = m_rtvDescHeap.Initialize(64, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_device.Get(), false); !res) {
        return CAVE_ERROR(res.error());
    }
    if (auto res = m_dsvDescHeap.Initialize(64, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, m_device.Get(), false); !res) {
        return CAVE_ERROR(res.error());
    }
    if (auto res = m_srvDescHeap.Initialize(512, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_device.Get(), true); !res) {
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
    scd.Format = d3d::Convert(DEFAULT_SURFACE_FORMAT);
    scd.Stereo = FALSE;
    scd.SampleDesc = { 1, 0 };
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // how swap chain is to be used
    scd.BufferCount = NUM_FRAMES_IN_FLIGHT;             // back buffer count
    scd.Scaling = DXGI_SCALING_STRETCH;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

    IDXGISwapChain1* pSwapChain = nullptr;

    D3D_FAIL(m_factory->CreateSwapChainForHwnd(
                 m_graphicsCommandQueue.Get(),
                 static_cast<HWND>(display_manager->nativeWindow()),
                 &scd,
                 NULL,
                 NULL,
                 &pSwapChain),
             "Failed to create swapchain");

    m_swapChain.Attach(reinterpret_cast<IDXGISwapChain3*>(pSwapChain));

    m_swapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
    m_swapChainWaitObject = m_swapChain->GetFrameLatencyWaitableObject();

    return Result<void>();
}

auto D3d12RenderDevice::createRenderTarget(uint32_t p_width, uint32_t p_height) -> Result<void> {
    for (int32_t i = 0; i < NUM_FRAMES_IN_FLIGHT; i++) {
        ID3D12Resource* backbuffer = nullptr;
        D3D_CALL(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backbuffer)));
        m_device->CreateRenderTargetView(backbuffer, nullptr, m_renderTargetDescriptor[i]);
        std::wstring name = std::wstring(L"Render Target Buffer") + std::to_wstring(i);
        backbuffer->SetName(name.c_str());
        m_renderTargets[i] = backbuffer;
    }

    D3D12_CLEAR_VALUE depthOptimizedClearValue{};
    depthOptimizedClearValue.Format = d3d::Convert(DEFAULT_DEPTH_STENCIL_FORMAT);
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
    resource_desc.Format = d3d::Convert(DEFAULT_DEPTH_STENCIL_FORMAT);
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
            d3d::Convert(p_desc.minFilter, p_desc.magFilter),
            d3d::Convert(p_desc.addressU),
            d3d::Convert(p_desc.addressV),
            d3d::Convert(p_desc.addressW),
            p_desc.mipLodBias,
            p_desc.maxAnisotropy,
            d3d::Convert(p_desc.comparisonFunc),
            d3d::Convert(p_desc.staticBorderColor),
            p_desc.minLod,
            p_desc.maxLod);

        return sampler_desc;
    };

#define SAMPLER_STATE(REG, NAME, DESC) m_staticSamplers.emplace_back(FillSamplerDesc(REG, DESC));
#include "sampler.slang.h"
#undef SAMPLER_STATE
}

auto D3d12RenderDevice::createRootSignature() -> Result<void> {
    // @TODO: Order from most frequent to least frequent.
    CD3DX12_ROOT_PARAMETER params[16]{};
    int idx = 0;

    params[idx++].InitAsConstantBufferView(0);
    params[idx++].InitAsConstantBufferView(1);
    params[idx++].InitAsConstantBufferView(2);
    params[idx++].InitAsConstantBufferView(3);
    params[idx++].InitAsConstantBufferView(4);
    params[idx++].InitAsConstantBufferView(5);
    params[idx++].InitAsConstantBufferView(6);

    CD3DX12_DESCRIPTOR_RANGE srvRange0;
    srvRange0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 32, 0, 0);  // t0..t31 space0

    CD3DX12_DESCRIPTOR_RANGE uavRange0;
    uavRange0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 16, 0, 0);  // u0..u15 space0

    CD3DX12_DESCRIPTOR_RANGE srvRangeBindless;
    srvRangeBindless.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 512, 0, 1);  // t0..t511 space1

    CD3DX12_DESCRIPTOR_RANGE uavRangeBindless;
    uavRangeBindless.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 64, 0, 1);  // u0..u63 space1

    params[idx++].InitAsDescriptorTable(1, &srvRange0);         // Fixed SRVs
    params[idx++].InitAsDescriptorTable(1, &uavRange0);         // Fixed UAVs
    params[idx++].InitAsDescriptorTable(1, &srvRangeBindless);  // Bindless SRVs
    params[idx++].InitAsDescriptorTable(1, &uavRangeBindless);  // Bindless UAVs

    initStaticSamplers();

    CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc(idx,
                                                    params,
                                                    (uint32_t)m_staticSamplers.size(),
                                                    m_staticSamplers.data(),
                                                    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    // @TODO: print error
    HRESULT hr = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    if (FAILED(hr)) {
        char buffer[256]{ 0 };
        StringUtils::sprintf(buffer, "%.*s", error->GetBufferSize(), error->GetBufferPointer());
        LOG_ERROR("Failed to create root signature, reason {}", buffer);
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "Failed to create root signature");
    }

    D3D_FAIL(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)),
             "Failed to create root signature");

    return Result<void>();
}

void D3d12RenderDevice::onWindowResize(int p_width, int p_height) {
    if (m_swapChain) {
        cleanupRenderTarget();
        D3D_CALL(m_swapChain->ResizeBuffers(0, p_width, p_height,
                                            DXGI_FORMAT_UNKNOWN,
                                            DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT));

        [[maybe_unused]] auto err = createRenderTarget(p_width, p_height);
    }
}

void D3d12RenderDevice::setPipelineStateImpl(PipelineStateName name) {
    auto pipeline = reinterpret_cast<D3d12PipelineState*>(m_pipeline_state_manager->findPSO(name));
    DEV_ASSERT(pipeline);

    auto primitive_topology = d3d::Convert(pipeline->desc.primitive_topology);
    m_graphicsCommandList->IASetPrimitiveTopology(primitive_topology);

    m_graphicsCommandList->SetPipelineState(pipeline->pso.Get());
}

void D3d12RenderDevice::cleanupRenderTarget() {
    flushGraphicsContext();

    for (uint32_t i = 0; i < NUM_BACK_BUFFERS; ++i) {
        SafeRelease(m_renderTargets[i]);
    }
    SafeRelease(m_depthStencilBuffer);
}

}  // namespace cave::render
