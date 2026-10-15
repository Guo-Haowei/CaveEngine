#pragma once
#include <d3d12.h>
#include <wrl/client.h>

#include "cave/render/rhi/ICommandList.h"

namespace cave::render {

class D3d12PipelineStateManager;

class D3d12CommandList final : public ICommandList {
public:
    explicit D3d12CommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list,
                              D3d12PipelineStateManager& pso_manager)
        : m_command_list(command_list)
        , m_pipeline_state_manager(pso_manager) {}

    void draw(uint32_t count,
              uint32_t offset) override;

    void drawInstanced(uint32_t instance_count,
                       uint32_t count,
                       uint32_t offset) override;

    void drawIndexed(uint32_t count,
                     uint32_t offset) override;

    void drawIndexedInstanced(uint32_t instance_count,
                              uint32_t count,
                              uint32_t offset) override;

    void bindConstantBufferRange(const GpuConstantBuffer* buffer,
                                 uint32_t size,
                                 uint32_t offset) override;

    void setPipelineState(PipelineStateName name) override;

    void setStencilRef(uint32_t ref) override;

    void dispatch(uint32_t num_groups_x,
                  uint32_t num_groups_y,
                  uint32_t num_groups_z) override;

    // @TODO: remove these
    void setMesh(const GpuMesh* mesh) override;

    void bindTexture(Dimension dimension, uint64_t handle, int slot) override;
    void unbindTexture(Dimension dimension, int slot) override;

    void bindStructuredBuffer(int slot, const GpuStructuredBuffer* buffer) override;
    void unbindStructuredBuffer(int slot) override;
    void bindStructuredBufferSRV(int slot, const GpuStructuredBuffer* buffer) override;
    void unbindStructuredBufferSRV(int slot) override;

    void bindUnorderedAccessView(uint32_t slot, GpuTexture* texture) override;
    void unbindUnorderedAccessView(uint32_t slot) override;

private:
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command_list;

    D3d12PipelineStateManager& m_pipeline_state_manager;
};

}  // namespace cave::render
