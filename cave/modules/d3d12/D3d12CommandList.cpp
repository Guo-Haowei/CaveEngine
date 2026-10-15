#include "D3d12CommandList.h"

#include "D3d12PipelineStateManager.h"
#include "D3d12Resources.h"

// @TODO: refactor
#define INCLUDE_AS_D3D12
#include "../d3d_common/D3dConvert.h"

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

void D3d12CommandList::bindConstantBufferRange(const GpuConstantBuffer* buffer,
                                               uint32_t size,
                                               uint32_t offset) {

    auto d3d_buffer = reinterpret_cast<const D3d12ConstantBuffer*>(buffer);
    DEV_ASSERT(size + offset <= d3d_buffer->capacity);

    D3D12_GPU_VIRTUAL_ADDRESS batch_address = d3d_buffer->buffer->GetGPUVirtualAddress();

    batch_address += offset;
    m_command_list->SetGraphicsRootConstantBufferView(d3d_buffer->desc.slot, batch_address);
    m_command_list->SetComputeRootConstantBufferView(d3d_buffer->desc.slot, batch_address);
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

void D3d12CommandList::setMesh(const GpuMesh* gpu_mesh) {
    if (!gpu_mesh) {
        m_command_list->IASetVertexBuffers(0, 0, nullptr);
        m_command_list->IASetIndexBuffer(nullptr);
        return;
    }

    auto mesh = reinterpret_cast<const D3d12MeshBuffers*>(gpu_mesh);

    m_command_list->IASetVertexBuffers(0,
                                       mesh->desc.enabledVertexCount,
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

void D3d12CommandList::bindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) {
    unused(slot);
    unused(buffer);
}

void D3d12CommandList::unbindStructuredBuffer(int slot) {
    unused(slot);
}

void D3d12CommandList::bindStructuredBufferSRV(int slot, const GpuStructuredBuffer* buffer) {
    unused(slot);

    DEV_ASSERT(buffer);
}

void D3d12CommandList::unbindStructuredBufferSRV(int slot) {
    unused(slot);
}

}  // namespace cave::render
