#include "D3d11CommandList.h"

#include "D3d11PipelineStateManager.h"
#include "D3d11Resources.h"
#include "D3D11ViewCache.h"

// @TODO: refactor
#define INCLUDE_AS_D3D11
#include "../d3d_common/D3dConvert.h"

namespace cave::render {

static constexpr size_t kMaxRenderTargets = 8;

void D3d11CommandList::draw(uint32_t count,
                            uint32_t offset) {
    m_device_context->Draw(count, offset);
}

void D3d11CommandList::drawInstanced(uint32_t instance_count,
                                     uint32_t count,
                                     uint32_t offset) {
    m_device_context->DrawInstanced(count, instance_count, offset, 0);
}

void D3d11CommandList::drawIndexed(uint32_t count,
                                   uint32_t offset) {
    m_device_context->DrawIndexed(count, offset, 0);
}

void D3d11CommandList::drawIndexedInstanced(uint32_t instance_count,
                                            uint32_t count,
                                            uint32_t offset) {
    m_device_context->DrawIndexedInstanced(count, instance_count, offset, 0, 0);
}

void D3d11CommandList::beginRendering(const RenderingInfo& info) {
    DEV_ASSERT(info.colors.size() <= kMaxRenderTargets);

    ID3D11RenderTargetView* rtvs[kMaxRenderTargets]{ nullptr };
    ID3D11DepthStencilView* dsv = nullptr;

    uint32_t rtv_count = 0;
    for (const ColorAttachmentDesc& color : info.colors) {
        rtvs[rtv_count++] = m_view_cache.getOrCreateRtv(color);
    }

    if (info.depth) {
        dsv = m_view_cache.getOrCreateDsv(*info.depth);
    }

    m_device_context->OMSetRenderTargets(rtv_count, rtvs, dsv);

    // clear render targets
    for (uint32_t idx = 0; idx < rtv_count; ++idx) {
        const ColorAttachmentDesc& desc = info.colors[idx];
        if (desc.load == LoadOp::Clear) {
            m_device_context->ClearRenderTargetView(rtvs[idx], desc.clear_color);
        }
    }
    if (dsv) {
        const DepthAttachmentDesc& desc = *info.depth;
        uint32_t clear_flag = 0;
        if (desc.depth_load == LoadOp::Clear) clear_flag |= D3D11_CLEAR_DEPTH;
        if (desc.stencil_load == LoadOp::Clear) clear_flag |= D3D11_CLEAR_STENCIL;
        if (clear_flag) {
            m_device_context->ClearDepthStencilView(dsv, clear_flag, desc.clear_depth, desc.clear_stencil);
        }
    }
}

void D3d11CommandList::endRendering() {
    ID3D11RenderTargetView* rtvs[kMaxRenderTargets]{ nullptr };
    m_device_context->OMSetRenderTargets(std::size(rtvs), rtvs, nullptr);
}

void D3d11CommandList::pushDebugGroup(std::string_view name) {
    if (m_annotation) {
        std::wstring name_wstr(name.begin(), name.end());
        m_annotation->BeginEvent(name_wstr.c_str());
    }
}

void D3d11CommandList::popDebugGroup() {
    if (m_annotation) {
        m_annotation->EndEvent();
    }
}

void D3d11CommandList::bindConstantBufferRange(const GpuResource* buffer,
                                               uint32_t size,
                                               uint32_t offset) {
    auto d3d_buffer = reinterpret_cast<const D3d11Buffer*>(buffer);
    DEV_ASSERT(size + offset <= d3d_buffer->desc.byteSize());
    D3D11_MAPPED_SUBRESOURCE mapped;
    ZeroMemory(&mapped, sizeof(D3D11_MAPPED_SUBRESOURCE));
    if (d3d_buffer->data) {
        m_device_context->Map(d3d_buffer->internal_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, d3d_buffer->data + offset, size);
        m_device_context->Unmap(d3d_buffer->internal_buffer.Get(), 0);
    }
}

void D3d11CommandList::setStencilRef(uint32_t ref) {
    if (m_state_cache.depthStencil) {
        if (m_state_cache.stencilRef != ref) {
            m_device_context->OMSetDepthStencilState(m_state_cache.depthStencil, ref);
            m_state_cache.stencilRef = ref;
        }
    }
}

void D3d11CommandList::setViewport(const Viewport& viewport) {
    D3D11_VIEWPORT vp{
        .TopLeftX = static_cast<float>(viewport.top_left_x),
        .TopLeftY = static_cast<float>(viewport.top_left_y),
        .Width = static_cast<float>(viewport.width),
        .Height = static_cast<float>(viewport.height),
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f,
    };

    m_device_context->RSSetViewports(1, &vp);
}

void D3d11CommandList::setPipelineState(PipelineStateName name) {
    auto pipeline = reinterpret_cast<D3d11PipelineState*>(m_pipeline_state_manager.findPSO(name));
    DEV_ASSERT(pipeline);
    if (pipeline->cs) {
        m_device_context->CSSetShader(pipeline->cs.Get(), nullptr, 0);
        return;
    }

    m_device_context->VSSetShader(pipeline->vs.Get(), 0, 0);
    m_device_context->IASetInputLayout(pipeline->input_layout.Get());
    m_device_context->PSSetShader(pipeline->ps.Get(), 0, 0);

    if (pipeline->rasterizer_state.Get() != m_state_cache.rasterizer) {
        m_device_context->RSSetState(pipeline->rasterizer_state.Get());
        m_state_cache.rasterizer = pipeline->rasterizer_state.Get();
    }
    if (pipeline->depth_stencil_state.Get() != m_state_cache.depthStencil) {
        m_device_context->OMSetDepthStencilState(pipeline->depth_stencil_state.Get(), 0);
        m_state_cache.depthStencil = pipeline->depth_stencil_state.Get();
    }
    if (pipeline->blend_state.Get() != m_state_cache.blendState) {
        // @TODO: remove hard code mask
        m_device_context->OMSetBlendState(pipeline->blend_state.Get(), nullptr, 0xFFFFFFFF);
        m_state_cache.blendState = pipeline->blend_state.Get();
    }

    auto topology = d3d::Convert(pipeline->desc.primitive_topology);
    m_device_context->IASetPrimitiveTopology(topology);
}

void D3d11CommandList::dispatch(uint32_t num_groups_x,
                                uint32_t num_groups_y,
                                uint32_t num_groups_z) {
    m_device_context->Dispatch(num_groups_x, num_groups_y, num_groups_z);
}

void D3d11CommandList::setMesh(const GpuMesh* mesh) {
    if (!mesh) {
        m_device_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
        m_device_context->IASetInputLayout(nullptr);
        return;
    }
    auto d3d_mesh = reinterpret_cast<const D3d11MeshBuffers*>(mesh);

    std::array<ID3D11Buffer*, kMeshMaxVertexBufferCount> buffers{ nullptr };
    std::array<uint32_t, kMeshMaxVertexBufferCount> strides{ 0 };
    std::array<uint32_t, kMeshMaxVertexBufferCount> offsets{ 0 };

    for (int index = 0; index < d3d_mesh->vertexBuffers.size(); ++index) {
        const auto& vertex = d3d_mesh->vertexBuffers[index];
        if (vertex == nullptr) {
            continue;
        }
        buffers[index] = (ID3D11Buffer*)vertex->getHandle();
        strides[index] = mesh->desc.vertex_layout[index].stride_in_byte;
        offsets[index] = mesh->desc.vertex_layout[index].offset_in_byte;
    }

    m_device_context->IASetVertexBuffers(0, kMeshMaxVertexBufferCount, buffers.data(), strides.data(), offsets.data());

    if (d3d_mesh->indexBuffer) {
        ID3D11Buffer* index_buffer = (ID3D11Buffer*)d3d_mesh->indexBuffer->getHandle();
        m_device_context->IASetIndexBuffer(index_buffer, DXGI_FORMAT_R32_UINT, 0);
    }
}

void D3d11CommandList::bindTexture(Dimension, uint64_t handle, int slot) {
    if (handle) {
        ID3D11ShaderResourceView* srv = (ID3D11ShaderResourceView*)(handle);
        m_device_context->PSSetShaderResources(slot, 1, &srv);
        m_device_context->CSSetShaderResources(slot, 1, &srv);
    }
}

void D3d11CommandList::unbindTexture(Dimension, int slot) {
    ID3D11ShaderResourceView* srv = nullptr;
    m_device_context->PSSetShaderResources(slot, 1, &srv);
    m_device_context->CSSetShaderResources(slot, 1, &srv);
}

void D3d11CommandList::bindUnorderedAccessView(uint32_t p_slot, GpuResource* p_texture) {
    DEV_ASSERT(p_texture);

    ID3D11UnorderedAccessView* ptr = reinterpret_cast<ID3D11UnorderedAccessView*>(p_texture->getUavHandle());
    m_device_context->CSSetUnorderedAccessViews(p_slot, 1, &ptr, nullptr);
}

void D3d11CommandList::unbindUnorderedAccessView(uint32_t p_slot) {
    ID3D11UnorderedAccessView* uav = nullptr;
    m_device_context->CSSetUnorderedAccessViews(p_slot, 1, &uav, nullptr);
}

}  // namespace cave::render