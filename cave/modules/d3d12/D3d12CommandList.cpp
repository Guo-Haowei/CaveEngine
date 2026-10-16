#include "D3d12CommandList.h"

#include "D3d12PipelineStateManager.h"
#include "D3d12Resources.h"
#include "D3d12ViewCache.h"

// @TODO: refactor
#define INCLUDE_AS_D3D12
#include "../d3d_common/D3dConvert.h"

#include <pix.h>

namespace cave::render {

void D3d12CommandList::draw(uint32_t count, uint32_t offset) {
    m_command_list->DrawInstanced(count, 1, offset, 0);
}

void D3d12CommandList::drawInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) {
    m_command_list->DrawInstanced(count, instance_count, offset, 0);
}

void D3d12CommandList::drawIndexed(uint32_t count, uint32_t offset) {
    m_command_list->DrawIndexedInstanced(count, 1, offset, 0, 0);
}

void D3d12CommandList::drawIndexedInstanced(uint32_t instance_count, uint32_t count, uint32_t offset) {
    m_command_list->DrawIndexedInstanced(count, instance_count, offset, 0, 0);
}

void D3d12CommandList::beginRendering(const RenderingInfo& info) {
    constexpr size_t kMaxRenderTargets = 8;
    DEV_ASSERT(info.colors.size() <= kMaxRenderTargets);

    std::array<D3D12_CPU_DESCRIPTOR_HANDLE, kMaxRenderTargets> rtvs{};
    uint32_t rtv_count = 0;

    // 1. Transition and collect color attachment RTVs
    for (const auto& color : info.colors) {
        if (!color.tex) continue;
        auto* d3d_tex = static_cast<D3d12GpuTexture*>(color.tex.get());

        transitionTexture(d3d_tex, D3D12_RESOURCE_STATE_RENDER_TARGET);

        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = m_view_cache.getOrCreateRtv(color);
        rtvs[rtv_count++] = rtv_handle;

        if (color.load == LoadOp::Clear) {
            m_command_list->ClearRenderTargetView(rtv_handle, color.clear_color, 0, nullptr);
        }
    }

    // 2. Transition and collect depth attachment DSV
    D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle{};
    const D3D12_CPU_DESCRIPTOR_HANDLE* dsv_ptr = nullptr;

    if (info.depth && info.depth->tex) {
        auto* d3d_depth = static_cast<D3d12GpuTexture*>(info.depth->tex.get());
        transitionTexture(d3d_depth, D3D12_RESOURCE_STATE_DEPTH_WRITE);

        dsv_handle = m_view_cache.getOrCreateDsv(*info.depth);
        dsv_ptr = &dsv_handle;

        UINT clear_flags = 0;
        if (info.depth->depth_load == LoadOp::Clear) clear_flags |= D3D12_CLEAR_FLAG_DEPTH;
        if (info.depth->stencil_load == LoadOp::Clear) clear_flags |= D3D12_CLEAR_FLAG_STENCIL;

        if (clear_flags) {
            m_command_list->ClearDepthStencilView(dsv_handle,
                                                  (D3D12_CLEAR_FLAGS)clear_flags,
                                                  info.depth->clear_depth,
                                                  info.depth->clear_stencil,
                                                  0, nullptr);
        }
    }

    // 3. Bind targets
    m_command_list->OMSetRenderTargets(rtv_count, rtvs.data(), false, dsv_ptr);
}

void D3d12CommandList::endRendering() {
    m_command_list->OMSetRenderTargets(0, nullptr, false, nullptr);
}

void D3d12CommandList::pushDebugGroup(std::string_view name) {
    std::wstring name_wstr(name.begin(), name.end());
    PIXBeginEvent(m_command_list.Get(), PIX_COLOR_DEFAULT, name_wstr.c_str());
}

void D3d12CommandList::popDebugGroup() {
    PIXEndEvent(m_command_list.Get());
}

void D3d12CommandList::bindConstantBufferRange(const GpuResource* buffer,
                                               uint32_t size,
                                               uint32_t offset) {

    auto d3d_buffer = reinterpret_cast<const D3d12GpuBuffer*>(buffer);
    DEV_ASSERT(size + offset <= d3d_buffer->desc.byteSize());

    D3D12_GPU_VIRTUAL_ADDRESS batch_address = d3d_buffer->buffer->GetGPUVirtualAddress();

    batch_address += offset;
    m_command_list->SetGraphicsRootConstantBufferView(d3d_buffer->desc.deprecated_slot, batch_address);
    m_command_list->SetComputeRootConstantBufferView(d3d_buffer->desc.deprecated_slot, batch_address);
}

void D3d12CommandList::setViewport(const Viewport& viewport) {
    D3D12_VIEWPORT vp{
        .TopLeftX = static_cast<float>(viewport.top_left_x),
        .TopLeftY = static_cast<float>(viewport.top_left_y),
        .Width = static_cast<float>(viewport.width),
        .Height = static_cast<float>(viewport.height),
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f,
    };

    D3D12_RECT rect{
        .left = 0,
        .top = 0,
        .right = viewport.top_left_x + viewport.width,
        .bottom = viewport.top_left_y + viewport.height,
    };

    m_command_list->RSSetViewports(1, &vp);
    m_command_list->RSSetScissorRects(1, &rect);
}

void D3d12CommandList::setStencilRef(uint32_t ref) {
    m_command_list->OMSetStencilRef(ref);
}

void D3d12CommandList::setPipelineState(PipelineStateName name) {
    auto pipeline = reinterpret_cast<D3d12PipelineState*>(m_pipeline_state_manager.findPSO(name));
    DEV_ASSERT(pipeline);

    auto primitive_topology = d3d::Convert(pipeline->desc.primitive_topology);
    m_command_list->IASetPrimitiveTopology(primitive_topology);

    m_command_list->SetPipelineState(pipeline->pso.Get());
}

void D3d12CommandList::dispatch(uint32_t num_groups_x,
                                uint32_t num_groups_y,
                                uint32_t num_groups_z) {
    m_command_list->Dispatch(num_groups_x, num_groups_y, num_groups_z);
}

void D3d12CommandList::barriers(std::span<const Ref<GpuResource>> resources) {
    if (resources.empty()) return;

    std::vector<D3D12_RESOURCE_BARRIER> d3d_barriers;
    d3d_barriers.reserve(resources.size());

    for (auto& res : resources) {
        if (!res) continue;

        ID3D12Resource* native_res = nullptr;
        D3D12_RESOURCE_STATES state;
        if (res->desc.isBuffer()) {
            auto ptr = static_cast<D3d12GpuBuffer*>(res.get());
            native_res = ptr->buffer.Get();
            state = ptr->current_state;
        } else {
            auto ptr = static_cast<D3d12GpuTexture*>(res.get());
            native_res = ptr->texture.Get();
            state = ptr->current_state;
            continue;
        }
        if (!native_res) continue;

        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = native_res;
        barrier.Transition.StateBefore = state;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        if (barrier.Transition.StateBefore == barrier.Transition.StateAfter) {
            continue;
        }

        d3d_barriers.push_back(barrier);
    }

    if (!d3d_barriers.empty()) {
        m_command_list->ResourceBarrier(static_cast<UINT>(d3d_barriers.size()), d3d_barriers.data());
    }
}

void D3d12CommandList::setMesh(const GpuMesh* gpu_mesh) {
    if (!gpu_mesh) {
        m_command_list->IASetVertexBuffers(0, 0, nullptr);
        m_command_list->IASetIndexBuffer(nullptr);
        return;
    }

    auto mesh = reinterpret_cast<const D3d12MeshBuffers*>(gpu_mesh);

    m_command_list->IASetVertexBuffers(0,
                                       mesh->desc.enabled_vertex_count,
                                       mesh->vbvs);
    if (mesh->indexBuffer) {
        m_command_list->IASetIndexBuffer(&mesh->ibv);
    }
}

void D3d12CommandList::bindTexture(Dimension, uint64_t handle, int slot) {
    unused(handle);
    unused(slot);
}

void D3d12CommandList::unbindTexture(Dimension, int slot) {
    unused(slot);
}

void D3d12CommandList::bindUnorderedAccessView(uint32_t slot, GpuResource* texture) {
    unused(slot);
    unused(texture);
}

void D3d12CommandList::unbindUnorderedAccessView(uint32_t slot) {
    unused(slot);
}

[[maybe_unused]] static const char* D3D12ResourceStateToString(D3D12_RESOURCE_STATES state) {
    switch (state) {
        case D3D12_RESOURCE_STATE_COMMON:
            return "COMMON / PRESENT";
        case D3D12_RESOURCE_STATE_RENDER_TARGET:
            return "RENDER_TARGET";
        case D3D12_RESOURCE_STATE_DEPTH_WRITE:
            return "DEPTH_WRITE";
        case D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE:
            return "PIXEL_SHADER_RESOURCE";
        case D3D12_RESOURCE_STATE_COPY_DEST:
            return "COPY_DEST";
        case D3D12_RESOURCE_STATE_COPY_SOURCE:
            return "COPY_SOURCE";
        case D3D12_RESOURCE_STATE_UNORDERED_ACCESS:
            return "UNORDERED_ACCESS";
        default:
            return "OTHER / COMBINED";
    }
}

void D3d12CommandList::transitionTexture(D3d12GpuTexture* texture, D3D12_RESOURCE_STATES target_state) {
    if (!texture || !texture->texture) {
        return;
    }

    if (texture->current_state != target_state) {
#if 0
        LOG_INFO(LogChannel::RHI,
                 "[Barrier] Texture '{}' (0x{:x}) | {} -> {}",
                 texture->desc.name.empty() ? "Unnamed" : texture->desc.name.c_str(),
                 reinterpret_cast<uintptr_t>(texture->texture.Get()),
                 D3D12ResourceStateToString(texture->currentState),
                 D3D12ResourceStateToString(target_state));
#endif

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            texture->texture.Get(),
            texture->current_state,
            target_state);
        m_command_list->ResourceBarrier(1, &barrier);
        texture->current_state = target_state;
    }
}

}  // namespace cave::render
