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
    explicit D3d12PipelineStateManager(IRenderDevice* p_graphics_manager) noexcept;

protected:
    auto graphicsPipeline(const PipelineStateDesc& p_desc) -> Result<std::shared_ptr<PipelineState>> final;
    auto computePipeline(const PipelineStateDesc& p_desc) -> Result<std::shared_ptr<PipelineState>> final;

private:
    IRenderDevice* m_device;
    std::vector<D3D_SHADER_MACRO> m_defines;
};

}  // namespace cave::render
