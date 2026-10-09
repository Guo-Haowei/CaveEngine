#include "d3d11_pipeline_state_manager.h"

#include "cave/runtime/framework/IApplication.h"

#include "../d3d_common/d3d_common.h"
#include "d3d11_graphics_manager.h"
#include "d3d11_helpers.h"
#define INCLUDE_AS_D3D11
#include "../d3d_common/d3d_convert.h"

namespace cave::render {

using Microsoft::WRL::ComPtr;

D3d11PipelineStateManager::D3d11PipelineStateManager(IRenderDevice* device) noexcept
    : PipelineStateManager(Backend::Direct3D11)
    , m_device(device) {
    m_defines.push_back({ "HLSL_LANG", "1" });
    m_defines.push_back({ "HLSL_LANG_D3D11", "1" });
    m_defines.push_back({ nullptr, nullptr });
}

auto D3d11PipelineStateManager::graphicsPipeline(const PipelineStateDesc& pipeline_state_desc) -> Result<Owner<PipelineState>> {
    auto graphics_manager = reinterpret_cast<D3d11GraphicsManager*>(m_device);
    auto& device = graphics_manager->GetD3dDevice();
    DEV_ASSERT(device);
    if (!device) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA);
    }

    auto pipeline_state = MakeOwner<D3d11PipelineState>(pipeline_state_desc);

    HRESULT hr = S_OK;
    ComPtr<ID3DBlob> vsblob;
    if (!pipeline_state_desc.vs.empty()) {
        auto res = CompileShader(pipeline_state_desc.vs, "vs_5_0", m_defines.data());
        if (!res) {
            return CAVE_ERROR(res.error());
        }

        ComPtr<ID3DBlob> blob;
        blob = *res;
        hr = device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pipeline_state->vs.GetAddressOf());
        D3D_FAIL_V_MSG(hr, nullptr, "failed to create vertex shader");
        vsblob = blob;
    }
    if (!pipeline_state_desc.ps.empty()) {
        auto res = CompileShader(pipeline_state_desc.ps, "ps_5_0", m_defines.data());
        if (!res) {
            return CAVE_ERROR(res.error());
        }

        ComPtr<ID3DBlob> blob;
        blob = *res;
        hr = device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pipeline_state->ps.GetAddressOf());
        D3D_FAIL_V_MSG(hr, nullptr, "failed to create pixel shader");
    }

    if (pipeline_state_desc.input_layout_desc) {
        std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
        elements.reserve(pipeline_state_desc.input_layout_desc->elements.size());
        for (const auto& ele : pipeline_state_desc.input_layout_desc->elements) {
            D3D11_INPUT_ELEMENT_DESC element_desc;
            element_desc.SemanticName = ele.semantic_name.c_str();
            element_desc.SemanticIndex = ele.semantic_index;
            element_desc.Format = d3d::Convert(ele.format);
            element_desc.InputSlot = ele.input_slot;
            element_desc.AlignedByteOffset = ele.aligned_byte_offset;
            element_desc.InputSlotClass = d3d::Convert(ele.input_slot_class);
            element_desc.InstanceDataStepRate = ele.instance_data_step_rate;
            elements.emplace_back(element_desc);
        }
        DEV_ASSERT(elements.size());

        hr = device->CreateInputLayout(elements.data(), (UINT)elements.size(), vsblob->GetBufferPointer(), vsblob->GetBufferSize(), pipeline_state->input_layout.GetAddressOf());
        D3D_FAIL_V_MSG(hr, nullptr, "failed to create input layout");
    }

    if (DEV_VERIFY(pipeline_state_desc.rasterizer_desc)) {
        ComPtr<ID3D11RasterizerState> state;

        auto it = m_rasterizer_states.find(pipeline_state_desc.rasterizer_desc);
        if (it == m_rasterizer_states.end()) {
            D3D11_RASTERIZER_DESC rasterizer_desc{};
            rasterizer_desc.FillMode = d3d::Convert(pipeline_state_desc.rasterizer_desc->fillMode);
            rasterizer_desc.CullMode = d3d::Convert(pipeline_state_desc.rasterizer_desc->cullMode);
            rasterizer_desc.FrontCounterClockwise = pipeline_state_desc.rasterizer_desc->frontCounterClockwise;
            hr = device->CreateRasterizerState(&rasterizer_desc, state.GetAddressOf());
            D3D_FAIL_V_MSG(hr, nullptr, "failed to create rasterizer state");
            m_rasterizer_states[pipeline_state_desc.rasterizer_desc] = state;
        } else {
            state = it->second;
        }
        DEV_ASSERT(state);
        pipeline_state->rasterizer_state = state;
    }
    if (DEV_VERIFY(pipeline_state_desc.depth_stencil_desc)) {
        ComPtr<ID3D11DepthStencilState> state;

        auto it = m_depth_stencil_states.find(pipeline_state_desc.depth_stencil_desc);
        if (it == m_depth_stencil_states.end()) {
            D3D11_DEPTH_STENCIL_DESC desc = d3d::Convert(pipeline_state_desc.depth_stencil_desc);
            D3D_FAIL_V(device->CreateDepthStencilState(&desc, state.GetAddressOf()), nullptr);
            m_depth_stencil_states[pipeline_state_desc.depth_stencil_desc] = state;
        } else {
            state = it->second.Get();
        }
        DEV_ASSERT(state);
        pipeline_state->depth_stencil_state = state;
    }
    if (DEV_VERIFY(pipeline_state_desc.blend_desc)) {
        ComPtr<ID3D11BlendState> state;

        auto it = m_blend_states.find(pipeline_state_desc.blend_desc);
        if (it == m_blend_states.end()) {
            D3D11_BLEND_DESC desc = d3d::Convert(pipeline_state_desc.blend_desc);
            D3D_FAIL_V(device->CreateBlendState(&desc, state.GetAddressOf()), nullptr);
            m_blend_states[pipeline_state_desc.blend_desc] = state;
        } else {
            state = it->second.Get();
        }
        DEV_ASSERT(state);
        pipeline_state->blend_state = state;
    }

    return pipeline_state;
}

auto D3d11PipelineStateManager::computePipeline(const PipelineStateDesc& pipeline_state_desc) -> Result<Owner<PipelineState>> {
    auto graphics_manager = reinterpret_cast<D3d11GraphicsManager*>(RenderDevice::singletonPtr());
    auto& device = graphics_manager->GetD3dDevice();
    DEV_ASSERT(device);

    if (!device) {
        return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA);
    }

    auto pipeline_state = MakeOwner<D3d11PipelineState>(pipeline_state_desc);

    auto res = CompileShader(pipeline_state_desc.cs, "cs_5_0", m_defines.data());
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    ComPtr<ID3DBlob> blob;
    blob = *res;
    HRESULT hr = device->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pipeline_state->cs.GetAddressOf());

    D3D_FAIL_V_MSG(hr, CAVE_ERROR(res.error()), "failed to create vertex buffer");

    return pipeline_state;
}

}  // namespace cave::render

#undef INCLUDE_AS_D3D11
