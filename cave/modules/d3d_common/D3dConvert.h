#ifdef CONVERT_H_INCLUDED
#error DO NOT INCLUDE THIS IN A HEADER FILE
#endif
#define CONVERT_H_INCLUDED

#if !defined(INCLUDE_AS_D3D11) && !defined(INCLUDE_AS_D3D12)
#define INCLUDE_AS_D3D11
#endif  // !INCLUDE_AS_D3D11) && !defined(INCLUDE_AS_D3D12)

// @TODO: move to a common place
#if defined(INCLUDE_AS_D3D11)
#include <d3d11.h>
#define D3D_(a)                      D3D11_##a
#define D3D_INPUT_CLASSIFICATION_(a) D3D11_INPUT_##a
#define D3D_FILL_MODE_(a)            D3D11_FILL_##a
#define D3D_CULL_MODE_(a)            D3D11_CULL_##a
#define D3D_COMPARISON_(a)           D3D11_COMPARISON_##a
#define D3D_FILTER_(a)               D3D11_FILTER_##a
#define D3D_TEXTURE_ADDRESS_MODE_(a) D3D11_TEXTURE_ADDRESS_##a
#define D3D_STENCIL_OP_(a)           D3D11_STENCIL_OP_##a
#define D3D_BLEND_(a)                D3D11_BLEND_##a
#elif defined(INCLUDE_AS_D3D12)
#include <d3d12.h>
#define D3D_(a)                      D3D12_##a
#define D3D_INPUT_CLASSIFICATION_(a) D3D12_INPUT_CLASSIFICATION_##a
#define D3D_FILL_MODE_(a)            D3D12_FILL_MODE_##a
#define D3D_CULL_MODE_(a)            D3D12_CULL_MODE_##a
#define D3D_COMPARISON_(a)           D3D12_COMPARISON_FUNC_##a
#define D3D_FILTER_(a)               D3D12_FILTER_##a
#define D3D_TEXTURE_ADDRESS_MODE_(a) D3D12_TEXTURE_ADDRESS_MODE_##a
#define D3D_STENCIL_OP_(a)           D3D12_STENCIL_OP_##a
#define D3D_BLEND_(a)                D3D12_BLEND_##a
#else
#error "Unknown API"
#endif

#include "cave/render/rhi/PipelineState.h"

namespace cave::render::d3d {

static inline DXGI_FORMAT Convert(PixelFormat format) {
    switch (format) {
        case PixelFormat::UNKNOWN:
            return DXGI_FORMAT_UNKNOWN;
        case PixelFormat::R8_UNORM:
            return DXGI_FORMAT_R8_UNORM;
        case PixelFormat::R8G8_UINT:
            return DXGI_FORMAT_R8G8_UNORM;
        case PixelFormat::R8G8B8A8_UINT:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case PixelFormat::R8G8B8A8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case PixelFormat::R8G8B8A8_UNORM_SRGB:
            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case PixelFormat::R16_FLOAT:
            return DXGI_FORMAT_R16_FLOAT;
        case PixelFormat::R16G16_FLOAT:
            return DXGI_FORMAT_R16G16_FLOAT;
        case PixelFormat::R16G16B16_FLOAT:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case PixelFormat::R16G16B16A16_FLOAT:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case PixelFormat::R32_FLOAT:
            return DXGI_FORMAT_R32_FLOAT;
        case PixelFormat::R32G32_FLOAT:
            return DXGI_FORMAT_R32G32_FLOAT;
        case PixelFormat::R32G32B32_FLOAT:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case PixelFormat::R32G32B32A32_FLOAT:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case PixelFormat::R32G32_UINT:
            return DXGI_FORMAT_R32G32_UINT;
        case PixelFormat::R32G32B32_UINT:
            return DXGI_FORMAT_R32G32B32_UINT;
        case PixelFormat::R32G32B32A32_UINT:
            return DXGI_FORMAT_R32G32B32A32_UINT;
        case PixelFormat::R32G32_SINT:
            return DXGI_FORMAT_R32G32_SINT;
        case PixelFormat::R32G32B32_SINT:
            return DXGI_FORMAT_R32G32B32_SINT;
        case PixelFormat::R32G32B32A32_SINT:
            return DXGI_FORMAT_R32G32B32A32_SINT;
        case PixelFormat::R11G11B10_FLOAT:
            return DXGI_FORMAT_R11G11B10_FLOAT;
        case PixelFormat::D32_FLOAT:
            return DXGI_FORMAT_D32_FLOAT;
        case PixelFormat::R24G8_TYPELESS:
            return DXGI_FORMAT_R24G8_TYPELESS;
        case PixelFormat::R24_UNORM_X8_TYPELESS:
            return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        case PixelFormat::D24_UNORM_S8_UINT:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case PixelFormat::X24_TYPELESS_G8_UINT:
            return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
        case PixelFormat::R32G8X24_TYPELESS:
            return DXGI_FORMAT_R32G8X24_TYPELESS;
        case PixelFormat::D32_FLOAT_S8X24_UINT:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        default:
            CRASH_NOW();
            return DXGI_FORMAT_UNKNOWN;
    }
}

static DXGI_FORMAT ToDsvFormat(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_R32G8X24_TYPELESS:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        case DXGI_FORMAT_R32_TYPELESS:
            return DXGI_FORMAT_D32_FLOAT;
        case DXGI_FORMAT_R24G8_TYPELESS:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case DXGI_FORMAT_R16_TYPELESS:
            return DXGI_FORMAT_D16_UNORM;
        default:
            return format;  // Already a typed format (e.g., DXGI_FORMAT_D32_FLOAT)
    }
}

static inline D3D_(INPUT_CLASSIFICATION) Convert(InputClassification p_input_classification) {
    switch (p_input_classification) {
        case InputClassification::PerVertexData:
            return D3D_INPUT_CLASSIFICATION_(PER_VERTEX_DATA);
        case InputClassification::PerInstanceData:
            return D3D_INPUT_CLASSIFICATION_(PER_INSTANCE_DATA);
    }
    return D3D_INPUT_CLASSIFICATION_(PER_VERTEX_DATA);
}

static inline D3D_(FILL_MODE) Convert(FillMode p_fill_mode) {
    switch (p_fill_mode) {
        case FillMode::Solid:
            return D3D_FILL_MODE_(SOLID);
        case FillMode::Wireframe:
            return D3D_FILL_MODE_(WIREFRAME);
    }
    return D3D_FILL_MODE_(SOLID);
}

static inline D3D_(CULL_MODE) Convert(CullMode p_cull_mode) {
    switch (p_cull_mode) {
        case CullMode::None:
            return D3D_CULL_MODE_(NONE);
        case CullMode::Front:
            return D3D_CULL_MODE_(FRONT);
        case CullMode::Back:
            return D3D_CULL_MODE_(BACK);
    }
    return D3D_CULL_MODE_(NONE);
}

static inline D3D_(COMPARISON_FUNC) Convert(ComparisonFunc p_func) {
    switch (p_func) {
#define COMPARISON_FUNC_ENUM(ENUM, VALUE) \
    case ComparisonFunc::ENUM:            \
        return D3D_COMPARISON_(ENUM);
        COMPARISON_FUNC_ENUM_LIST
#undef COMPARISON_FUNC_ENUM
        default:
            CRASH_NOW();
            return D3D_COMPARISON_(NEVER);
    }
}

static inline D3D_(TEXTURE_ADDRESS_MODE) Convert(AddressMode p_mode) {
    switch (p_mode) {
        case AddressMode::Wrap:
            return D3D_TEXTURE_ADDRESS_MODE_(WRAP);
        case AddressMode::Clamp:
            return D3D_TEXTURE_ADDRESS_MODE_(CLAMP);
        case AddressMode::Border:
            return D3D_TEXTURE_ADDRESS_MODE_(BORDER);
        default:
            CRASH_NOW();
            return D3D_TEXTURE_ADDRESS_MODE_(WRAP);
    }
}

static inline D3D_(BLEND) Convert(Blend blend) {
    switch (blend) {
        case Blend::Zero:
            return D3D_BLEND_(ZERO);
        case Blend::One:
            return D3D_BLEND_(ONE);
        case Blend::SrcAlpha:
            return D3D_BLEND_(SRC_ALPHA);
        case Blend::InvSrcAlpha:
            return D3D_BLEND_(INV_SRC_ALPHA);
        default:
            CRASH_NOW();
            return D3D_BLEND_(ZERO);
    }
}

static inline D3D_(BLEND_OP) Convert(BlendOp blend_op) {
    switch (blend_op) {
        case BlendOp::Add:
            return D3D_BLEND_(OP_ADD);
        case BlendOp::Sub:
            return D3D_BLEND_(OP_SUBTRACT);
        default:
            CRASH_NOW();
            return D3D_BLEND_(OP_ADD);
    }
}

static inline D3D_(STENCIL_OP) Convert(StencilOp stencil_op) {
    switch (stencil_op) {
#define STENCIL_OP_ENUM(ENUM, VALUE) \
    case StencilOp::ENUM:            \
        return D3D_STENCIL_OP_(ENUM);
        STENCIL_OP_ENUM_LIST
#undef STENCIL_OP_ENUM
        default:
            CRASH_NOW();
            return D3D_STENCIL_OP_(KEEP);
    }
}

// @TODO: refactor this
static inline D3D_(FILTER) Convert(MinFilter p_min_filter, MagFilter p_mag_filter) {
    const bool min_linear = p_min_filter != MinFilter::Point;
    const bool mag_linear = p_mag_filter != MagFilter::Point;
    const bool mip_linear = true;

    const auto filter = static_cast<D3D_(FILTER)>((min_linear << 4) | (mag_linear << 2) | mip_linear);
    return filter;
}

static inline auto Convert(const StencilOpDesc* stencil_op_desc) {
    if (!stencil_op_desc) {
        CRASH_NOW_MSG("TODO: default");
    }

    D3D_(DEPTH_STENCILOP_DESC)
    desc{};
    desc.StencilFunc = Convert(stencil_op_desc->stencilFunc);
    desc.StencilFailOp = Convert(stencil_op_desc->stencilFailOp);
    desc.StencilDepthFailOp = Convert(stencil_op_desc->stencilDepthFailOp);
    desc.StencilPassOp = Convert(stencil_op_desc->stencilPassOp);
    return desc;
}

static inline auto Convert(const BlendDesc* blend_desc) {
    if (!blend_desc) {
        CRASH_NOW_MSG("TODO: default");
    }

    D3D_(BLEND_DESC)
    desc{};
    for (uint32_t i = 0; i < std::size(blend_desc->render_targets); ++i) {
        auto& out = desc.RenderTarget[i];
        const auto& in = blend_desc->render_targets[i];
        out.BlendEnable = in.blend_enabled;
        out.SrcBlend = out.SrcBlendAlpha = Convert(in.blend_src);
        out.DestBlend = out.DestBlendAlpha = Convert(in.blend_dest);
        out.BlendOp = out.BlendOpAlpha = Convert(in.blend_op);
        out.RenderTargetWriteMask = in.color_write_mask;
    }
    return desc;
}

static inline auto Convert(const DepthStencilDesc* depth_stencil_desc) {
    if (!depth_stencil_desc) {
        CRASH_NOW_MSG("TODO: default");
    }

    D3D_(DEPTH_STENCIL_DESC)
    desc{};
    desc.DepthEnable = depth_stencil_desc->depthEnabled;
    desc.DepthFunc = d3d::Convert(depth_stencil_desc->depthFunc);
    desc.DepthWriteMask = D3D_(DEPTH_WRITE_MASK_ALL);
    desc.StencilEnable = depth_stencil_desc->stencilEnabled;
    desc.StencilWriteMask = depth_stencil_desc->stencilWriteMask;
    desc.StencilReadMask = depth_stencil_desc->stencilReadMask;

    desc.FrontFace = d3d::Convert(&depth_stencil_desc->frontFace);
    desc.BackFace = d3d::Convert(&depth_stencil_desc->backFace);
    return desc;
}

#if defined(INCLUDE_AS_D3D11)
static inline D3D11_USAGE Convert(BufferUsage usage) {
    switch (usage) {
        case BufferUsage::Immutable:
            return D3D11_USAGE_IMMUTABLE;
        case BufferUsage::Dynamic:
            return D3D11_USAGE_DYNAMIC;
        case BufferUsage::Staging:
            return D3D11_USAGE_STAGING;
        case BufferUsage::Default:
            [[fallthrough]];
        default:
            return D3D11_USAGE_DEFAULT;
    }
}
#endif

#if defined(INCLUDE_AS_D3D12)
static inline D3D12_STATIC_BORDER_COLOR Convert(StaticBorderColor color) {
    switch (color) {
        case StaticBorderColor::TransparentBlack:
            return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        case StaticBorderColor::OpaqueBlack:
            return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
        case StaticBorderColor::OpaqueWhite:
            return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
        default:
            CRASH_NOW();
            return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    }
}
#endif

static inline D3D_PRIMITIVE_TOPOLOGY Convert(PrimitiveTopology topology) {
    switch (topology) {
        case PrimitiveTopology::Point:
            return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        case PrimitiveTopology::Line:
            return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case PrimitiveTopology::Triangle:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        default:
            CRASH_NOW();
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }
}

#if defined(INCLUDE_AS_D3D12)
static inline D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertToType(PrimitiveTopology topology) {
    switch (topology) {
        case PrimitiveTopology::Point:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        case PrimitiveTopology::Line:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case PrimitiveTopology::Triangle:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        default:
            CRASH_NOW();
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    }
}
#endif

#undef D3D_
#undef D3D_INPUT_CLASSIFICATION_
#undef D3D_FILL_MODE_
#undef D3D_CULL_MODE_
#undef D3D_COMPARISON_FUNC_
#undef D3D_FILTER_
#undef D3D_TEXTURE_ADDRESS_MODE_
#undef D3D_STENCIL_OP_
#undef D3D_BLEND_

}  // namespace cave::render::d3d
