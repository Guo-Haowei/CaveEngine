#pragma once
#include <d3d12.h>
#include <wrl/client.h>

#include "engine/private/runtime/framework/PipelineStateManager.h"

namespace cave::render {

struct D3d12PipelineState : public PipelineState {
    using PipelineState::PipelineState;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
};

class D3d12PipelineStateManager : public PipelineStateManager {
public:
    explicit D3d12PipelineStateManager(IRenderDevice* device) noexcept;

protected:
    auto graphicsPipeline(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>> final;
    auto computePipeline(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>> final;

private:
    IRenderDevice* m_device;
};

}  // namespace cave::render
