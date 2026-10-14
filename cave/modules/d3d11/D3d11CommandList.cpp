#include "D3d11CommandList.h"

#include "D3d11PipelineStateManager.h"
#include "D3d11Resources.h"

// @TODO: refactor
#define INCLUDE_AS_D3D11
#include "../d3d_common/D3dConvert.h"

namespace cave::render {

void D3d11CommandList::draw(uint32_t count,
                            uint32_t offset) {
    m_deviceContext->Draw(count, offset);
}

void D3d11CommandList::drawInstanced(uint32_t instance_count,
                                     uint32_t count,
                                     uint32_t offset) {
    m_deviceContext->DrawInstanced(count, instance_count, offset, 0);
}

void D3d11CommandList::drawIndexed(uint32_t count,
                                   uint32_t offset) {
    m_deviceContext->DrawIndexed(count, offset, 0);
}

void D3d11CommandList::drawIndexedInstanced(uint32_t instance_count,
                                            uint32_t count,
                                            uint32_t offset) {
    m_deviceContext->DrawIndexedInstanced(count, instance_count, offset, 0, 0);
}

void D3d11CommandList::setStencilRef(uint32_t ref) {
    if (m_stateCache.depthStencil) {
        if (m_stateCache.stencilRef != ref) {
            m_deviceContext->OMSetDepthStencilState(m_stateCache.depthStencil, ref);
            m_stateCache.stencilRef = ref;
        }
    }
}

void D3d11CommandList::setPipelineState(PipelineStateName name) {
    auto pipeline = reinterpret_cast<D3d11PipelineState*>(m_pipeline_state_manager.findPSO(name));
    DEV_ASSERT(pipeline);
    if (pipeline->cs) {
        m_deviceContext->CSSetShader(pipeline->cs.Get(), nullptr, 0);
        return;
    }

    m_deviceContext->VSSetShader(pipeline->vs.Get(), 0, 0);
    m_deviceContext->IASetInputLayout(pipeline->input_layout.Get());
    m_deviceContext->PSSetShader(pipeline->ps.Get(), 0, 0);

    if (pipeline->rasterizer_state.Get() != m_stateCache.rasterizer) {
        m_deviceContext->RSSetState(pipeline->rasterizer_state.Get());
        m_stateCache.rasterizer = pipeline->rasterizer_state.Get();
    }
    if (pipeline->depth_stencil_state.Get() != m_stateCache.depthStencil) {
        m_deviceContext->OMSetDepthStencilState(pipeline->depth_stencil_state.Get(), 0);
        m_stateCache.depthStencil = pipeline->depth_stencil_state.Get();
    }
    if (pipeline->blend_state.Get() != m_stateCache.blendState) {
        // @TODO: remove hard code mask
        m_deviceContext->OMSetBlendState(pipeline->blend_state.Get(), nullptr, 0xFFFFFFFF);
        m_stateCache.blendState = pipeline->blend_state.Get();
    }

    auto topology = d3d::Convert(pipeline->desc.primitive_topology);
    m_deviceContext->IASetPrimitiveTopology(topology);
}

void D3d11CommandList::bindTexture(Dimension, uint64_t handle, int slot) {
    if (handle) {
        ID3D11ShaderResourceView* srv = (ID3D11ShaderResourceView*)(handle);
        m_deviceContext->PSSetShaderResources(slot, 1, &srv);
        m_deviceContext->CSSetShaderResources(slot, 1, &srv);
    }
}

void D3d11CommandList::unbindTexture(Dimension, int slot) {
    ID3D11ShaderResourceView* srv = nullptr;
    m_deviceContext->PSSetShaderResources(slot, 1, &srv);
    m_deviceContext->CSSetShaderResources(slot, 1, &srv);
}

void D3d11CommandList::bindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) {
    auto structured_buffer = reinterpret_cast<const D3d11StructuredBuffer*>(buffer);
    m_deviceContext->CSSetUnorderedAccessViews(slot, 1, structured_buffer->uav.GetAddressOf(), nullptr);
}

void D3d11CommandList::unbindStructuredBuffer(int slot) {
    ID3D11UnorderedAccessView* uav = nullptr;
    m_deviceContext->CSSetUnorderedAccessViews(slot, 1, &uav, nullptr);
}

void D3d11CommandList::bindStructuredBufferSRV(int slot, const GpuStructuredBuffer* buffer) {
    DEV_ASSERT(buffer);

    auto structured_buffer = reinterpret_cast<const D3d11StructuredBuffer*>(buffer);
    if (structured_buffer->srv != nullptr) {
        m_deviceContext->VSSetShaderResources(slot, 1, structured_buffer->srv.GetAddressOf());
    }
}

void D3d11CommandList::unbindStructuredBufferSRV(int slot) {
    ID3D11ShaderResourceView* srv = nullptr;
    m_deviceContext->VSSetShaderResources(slot, 1, &srv);
}

}  // namespace cave::render