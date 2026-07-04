#include "d3d12_pipeline_state_manager.h"

#include "../d3d_common/d3d_common.h"
#include "d3d12_graphics_manager.h"
#define INCLUDE_AS_D3D12
#include "../d3d_common/d3d_convert.h"

namespace cave::render {

using Microsoft::WRL::ComPtr;

D3d12PipelineStateManager::D3d12PipelineStateManager(IRenderDevice* p_device) noexcept
    : PipelineStateManager(Backend::Direct3D12)
    , m_device(p_device) {
    m_defines.push_back({ "HLSL_LANG", "1" });
    m_defines.push_back({ "HLSL_LANG_D3D12", "1" });
    m_defines.push_back({ nullptr, nullptr });
}

auto D3d12PipelineStateManager::computePipeline(const PipelineStateDesc& p_desc) -> Result<std::shared_ptr<PipelineState>> {
    auto graphics_manager = reinterpret_cast<D3d12GraphicsManager*>(m_device);

    auto pipeline_state = std::make_shared<D3d12PipelineState>(p_desc);

    ComPtr<ID3DBlob> cs_blob;
    if (!p_desc.cs.empty()) {
        auto res = CompileShader(p_desc.cs, "cs_5_1", m_defines.data());
        if (!res) {
            return CAVE_ERROR(res.error());
        }
        cs_blob = *res;
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC pso_desc = {};
    pso_desc.pRootSignature = graphics_manager->GetRootSignature();
    pso_desc.CS = CD3DX12_SHADER_BYTECODE(cs_blob.Get());

    ID3D12Device4* device = reinterpret_cast<D3d12GraphicsManager*>(m_device)->GetDevice();
    D3D_FAIL_V(device->CreateComputePipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state->pso)), CAVE_ERROR(ErrorCode::ERR_CANT_CREATE));

    return pipeline_state;
}

auto D3d12PipelineStateManager::graphicsPipeline(const PipelineStateDesc& p_desc) -> Result<std::shared_ptr<PipelineState>> {
    auto graphics_manager = reinterpret_cast<D3d12GraphicsManager*>(m_device);

    auto pipeline_state = std::make_shared<D3d12PipelineState>(p_desc);
    ComPtr<ID3DBlob> vs_blob;
    ComPtr<ID3DBlob> ps_blob;

    if (!p_desc.vs.empty()) {
        auto res = CompileShader(p_desc.vs, "vs_5_1", m_defines.data());
        if (!res) {
            return CAVE_ERROR(res.error());
        }
        vs_blob = *res;
    }
    if (!p_desc.ps.empty()) {
        auto res = CompileShader(p_desc.ps, "ps_5_1", m_defines.data());
        if (!res) {
            return CAVE_ERROR(res.error());
        }
        ps_blob = *res;
    }

    std::vector<D3D12_INPUT_ELEMENT_DESC> elements;
    elements.reserve(p_desc.input_layout_desc->elements.size());
    for (const auto& ele : p_desc.input_layout_desc->elements) {
        D3D12_INPUT_ELEMENT_DESC ildesc;
        ildesc.SemanticName = ele.semantic_name.c_str();
        ildesc.SemanticIndex = ele.semantic_index;
        ildesc.Format = d3d::Convert(ele.format);
        ildesc.InputSlot = ele.input_slot;
        ildesc.AlignedByteOffset = ele.aligned_byte_offset;
        ildesc.InputSlotClass = d3d::Convert(ele.input_slot_class);
        ildesc.InstanceDataStepRate = ele.instance_data_step_rate;
        elements.push_back(ildesc);
    }
    DEV_ASSERT(elements.size());

    D3D12_RASTERIZER_DESC rasterizer_desc{};
    rasterizer_desc.FillMode = d3d::Convert(p_desc.rasterizer_desc->fillMode);
    rasterizer_desc.CullMode = d3d::Convert(p_desc.rasterizer_desc->cullMode);
    rasterizer_desc.FrontCounterClockwise = p_desc.rasterizer_desc->frontCounterClockwise;
    rasterizer_desc.DepthBias = p_desc.rasterizer_desc->depthBias;
    rasterizer_desc.SlopeScaledDepthBias = p_desc.rasterizer_desc->slopeScaledDepthBias;
    rasterizer_desc.DepthClipEnable = p_desc.rasterizer_desc->depthClipEnable;
    // rasterizer_desc.ScissorEnable = p_desc.rasterizerDesc->scissorEnable;
    rasterizer_desc.MultisampleEnable = p_desc.rasterizer_desc->multisampleEnable;
    rasterizer_desc.AntialiasedLineEnable = p_desc.rasterizer_desc->antialiasedLineEnable;

    D3D12_DEPTH_STENCIL_DESC depth_stencil_desc = d3d::Convert(p_desc.depth_stencil_desc);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
    pso_desc.pRootSignature = graphics_manager->GetRootSignature();
    pso_desc.VS = CD3DX12_SHADER_BYTECODE(vs_blob.Get());
    if (ps_blob) {
        pso_desc.PS = CD3DX12_SHADER_BYTECODE(ps_blob.Get());
    }
    pso_desc.BlendState = d3d::Convert(p_desc.blend_desc);
    pso_desc.SampleMask = UINT_MAX;
    pso_desc.RasterizerState = rasterizer_desc;
    pso_desc.DepthStencilState = depth_stencil_desc;
    pso_desc.InputLayout = { elements.data(), (uint32_t)elements.size() };
    pso_desc.PrimitiveTopologyType = d3d::ConvertToType(p_desc.primitive_topology);
    pso_desc.SampleDesc.Count = 1;

    pso_desc.NumRenderTargets = p_desc.num_render_targets;
    for (uint32_t index = 0; index < p_desc.num_render_targets; ++index) {
        pso_desc.RTVFormats[index] = d3d::Convert(p_desc.rtv_formats[index]);
    }
    pso_desc.DSVFormat = d3d::Convert(p_desc.dsv_format);

    ID3D12Device4* device = reinterpret_cast<D3d12GraphicsManager*>(m_device)->GetDevice();
    D3D_FAIL_V(device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state->pso)), nullptr);

    return pipeline_state;
}

}  // namespace cave::render

#undef INCLUDE_AS_D3D12
