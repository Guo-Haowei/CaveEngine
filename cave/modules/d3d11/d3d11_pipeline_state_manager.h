#pragma once
#include <d3d11.h>
#include <wrl/client.h>

#include "engine/private/runtime/framework/PipelineStateManager.h"

namespace cave::render {

struct D3d11PipelineState : public PipelineState {
    using PipelineState::PipelineState;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> gs;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
    Microsoft::WRL::ComPtr<ID3D11ComputeShader> cs;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;

    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_state;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_state;
    Microsoft::WRL::ComPtr<ID3D11BlendState> blend_state;
};

class D3d11PipelineStateManager : public PipelineStateManager {
public:
    explicit D3d11PipelineStateManager(IRenderDevice* device) noexcept;

protected:
    auto graphicsPipeline(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>> final;
    auto computePipeline(const PipelineStateDesc& desc) -> Result<Owner<PipelineState>> final;

    HashMap<const RasterizerDesc*, Microsoft::WRL::ComPtr<ID3D11RasterizerState>> m_rasterizer_states;
    HashMap<const DepthStencilDesc*, Microsoft::WRL::ComPtr<ID3D11DepthStencilState>> m_depth_stencil_states;
    HashMap<const BlendDesc*, Microsoft::WRL::ComPtr<ID3D11BlendState>> m_blend_states;

private:
    IRenderDevice* m_device;
    Vector<D3D_SHADER_MACRO> m_defines;
};

}  // namespace cave::render
