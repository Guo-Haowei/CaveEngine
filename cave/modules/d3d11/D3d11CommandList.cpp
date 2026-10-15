#include "D3d11CommandList.h"

#include "D3d11PipelineStateManager.h"
#include "D3d11Resources.h"

// @TODO: refactor
#define INCLUDE_AS_D3D11
#include "../d3d_common/D3dConvert.h"

namespace cave::render {

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

void D3d11CommandList::bindConstantBufferRange(const GpuConstantBuffer* buffer,
                                               uint32_t size,
                                               uint32_t offset) {
    auto d3d_buffer = reinterpret_cast<const D3d11UniformBuffer*>(buffer);
    DEV_ASSERT(size + offset <= d3d_buffer->capacity);
    D3D11_MAPPED_SUBRESOURCE mapped;
    ZeroMemory(&mapped, sizeof(D3D11_MAPPED_SUBRESOURCE));
    if (d3d_buffer->data) {
        m_device_context->Map(d3d_buffer->internalBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, d3d_buffer->data + offset, size);
        m_device_context->Unmap(d3d_buffer->internalBuffer.Get(), 0);
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

    std::array<ID3D11Buffer*, MESH_MAX_VERTEX_BUFFER_COUNT> buffers{ nullptr };
    std::array<uint32_t, MESH_MAX_VERTEX_BUFFER_COUNT> strides{ 0 };
    std::array<uint32_t, MESH_MAX_VERTEX_BUFFER_COUNT> offsets{ 0 };

    for (int index = 0; index < d3d_mesh->vertexBuffers.size(); ++index) {
        const auto& vertex = d3d_mesh->vertexBuffers[index];
        if (vertex == nullptr) {
            continue;
        }
        buffers[index] = (ID3D11Buffer*)vertex->GetHandle();
        strides[index] = mesh->desc.vertexLayout[index].strideInByte;
        offsets[index] = mesh->desc.vertexLayout[index].offsetInByte;
    }

    m_device_context->IASetVertexBuffers(0, MESH_MAX_VERTEX_BUFFER_COUNT, buffers.data(), strides.data(), offsets.data());

    if (d3d_mesh->indexBuffer) {
        ID3D11Buffer* index_buffer = (ID3D11Buffer*)d3d_mesh->indexBuffer->GetHandle();
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

void D3d11CommandList::bindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) {
    auto structured_buffer = reinterpret_cast<const D3d11StructuredBuffer*>(buffer);
    m_device_context->CSSetUnorderedAccessViews(slot, 1, structured_buffer->uav.GetAddressOf(), nullptr);
}

void D3d11CommandList::unbindStructuredBuffer(int slot) {
    ID3D11UnorderedAccessView* uav = nullptr;
    m_device_context->CSSetUnorderedAccessViews(slot, 1, &uav, nullptr);
}

void D3d11CommandList::bindStructuredBufferSRV(int slot, const GpuStructuredBuffer* buffer) {
    DEV_ASSERT(buffer);

    auto structured_buffer = reinterpret_cast<const D3d11StructuredBuffer*>(buffer);
    if (structured_buffer->srv != nullptr) {
        m_device_context->VSSetShaderResources(slot, 1, structured_buffer->srv.GetAddressOf());
    }
}

void D3d11CommandList::unbindStructuredBufferSRV(int slot) {
    ID3D11ShaderResourceView* srv = nullptr;
    m_device_context->VSSetShaderResources(slot, 1, &srv);
}

void D3d11CommandList::bindUnorderedAccessView(uint32_t p_slot, GpuTexture* p_texture) {
    DEV_ASSERT(p_texture);

    ID3D11UnorderedAccessView* ptr = reinterpret_cast<ID3D11UnorderedAccessView*>(p_texture->GetUavHandle());
    m_device_context->CSSetUnorderedAccessViews(p_slot, 1, &ptr, nullptr);
}

void D3d11CommandList::unbindUnorderedAccessView(uint32_t p_slot) {
    ID3D11UnorderedAccessView* uav = nullptr;
    m_device_context->CSSetUnorderedAccessViews(p_slot, 1, &uav, nullptr);
}

}  // namespace cave::render