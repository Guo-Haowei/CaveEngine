#pragma once

#include "cave/render/rhi/Sampler.h"
#include "engine/private/render/rhi/gpu_resource.h"

#import <Metal/Metal.h>

namespace cave::render {

constexpr uint32_t METAL_VERTEX_BUFFER_BASE = 16;

inline static MTLCompareFunction ToCompare(ComparisonFunc f) {
    switch (f) {
    case ComparisonFunc::NEVER:
        return MTLCompareFunctionNever;
    case ComparisonFunc::LESS:
        return MTLCompareFunctionLess;
    case ComparisonFunc::EQUAL:
        return MTLCompareFunctionEqual;
    case ComparisonFunc::LESS_EQUAL:
        return MTLCompareFunctionLessEqual;
    case ComparisonFunc::GREATER:
        return MTLCompareFunctionGreater;
    case ComparisonFunc::NOT_EQUAL:
        return MTLCompareFunctionNotEqual;
    case ComparisonFunc::GREATER_EQUAL:
        return MTLCompareFunctionGreaterEqual;
    case ComparisonFunc::ALWAYS:
        return MTLCompareFunctionAlways;
    default:
        return MTLCompareFunctionAlways;
    }
}

inline static MTLStencilOperation ToStencilOp(StencilOp op) {
    switch (op) {
    case StencilOp::KEEP:
        return MTLStencilOperationKeep;
    case StencilOp::REPLACE:
        return MTLStencilOperationReplace;
    default:
        return MTLStencilOperationKeep;
    }
}

inline static MTLPixelFormat ToMetalTextureFormat(PixelFormat format) {
    switch (format) {
    case PixelFormat::UNKNOWN:
        return MTLPixelFormatInvalid;
    case PixelFormat::R8_UINT:
        return MTLPixelFormatR8Uint;
    case PixelFormat::R8G8_UINT:
        return MTLPixelFormatRG8Uint;
    case PixelFormat::R8G8B8A8_UINT:
        return MTLPixelFormatRGBA8Uint;
    case PixelFormat::R8_UNORM:
        return MTLPixelFormatR8Unorm;
    case PixelFormat::R8G8B8A8_UNORM:
        return MTLPixelFormatRGBA8Unorm;
    case PixelFormat::R8G8B8A8_UNORM_SRGB:
        return MTLPixelFormatRGBA8Unorm_sRGB;
    case PixelFormat::R16_FLOAT:
        return MTLPixelFormatR16Float;
    case PixelFormat::R16G16_FLOAT:
        return MTLPixelFormatRG16Float;
    case PixelFormat::R16G16B16_FLOAT:
        return MTLPixelFormatRGB16Float;
    case PixelFormat::R16G16B16A16_FLOAT:
        return MTLPixelFormatRGBA16Float;
    case PixelFormat::R32_FLOAT:
        return MTLPixelFormatR32Float;
    case PixelFormat::R32G32_FLOAT:
        return MTLPixelFormatRG32Float;
    case PixelFormat::R32G32B32_FLOAT:
        return MTLPixelFormatRGB32Float;
    case PixelFormat::R32G32B32A32_FLOAT:
        return MTLPixelFormatRGBA32Float;
    case PixelFormat::R32G32_UINT:
        return MTLPixelFormatRG32Uint;
    case PixelFormat::R32G32B32_UINT:
        return MTLPixelFormatRGB32Uint;
    case PixelFormat::R32G32B32A32_UINT:
        return MTLPixelFormatRGBA32Sint;
    case PixelFormat::R32G32_SINT:
        return MTLPixelFormatRG32Sint;
    case PixelFormat::R32G32B32_SINT:
        return MTLPixelFormatRGB32Sint;
    case PixelFormat::R32G32B32A32_SINT:
        return MTLPixelFormatRGBA32Sint;
    case PixelFormat::R11G11B10_FLOAT:
        return MTLPixelFormatRG11B10Float;
    case PixelFormat::D32_FLOAT:
        return MTLPixelFormatDepth32Float;
    case PixelFormat::R24G8_TYPELESS:
        return MTLPixelFormatDepth24Unorm_Stencil8;
    case PixelFormat::R24_UNORM_X8_TYPELESS:
        return MTLPixelFormatDepth24Unorm_Stencil8;
    case PixelFormat::D24_UNORM_S8_UINT:
        return MTLPixelFormatDepth24Unorm_Stencil8;
    case PixelFormat::X24_TYPELESS_G8_UINT:
        return MTLPixelFormatX32_Stencil8;
    case PixelFormat::R32G8X24_TYPELESS:
        return MTLPixelFormatDepth32Float_Stencil8;
    case PixelFormat::D32_FLOAT_S8X24_UINT:
        return MTLPixelFormatDepth32Float_Stencil8;
    default:
        CRASH_NOW();
        return MTLPixelFormatInvalid;
    }
}

inline static MTLSamplerMinMagFilter ToMetalFilter(MinFilter filter) {
    switch (filter) {
    case MinFilter::Point:
        return MTLSamplerMinMagFilterNearest;
    case MinFilter::Linear:
    case MinFilter::LinearMipmapLinear:
        return MTLSamplerMinMagFilterLinear;
    default:
        return MTLSamplerMinMagFilterLinear;
    }
}

inline static MTLSamplerMinMagFilter ToMetalFilter(MagFilter filter) {
    switch (filter) {
    case MagFilter::Point:
        return MTLSamplerMinMagFilterNearest;
    case MagFilter::Linear:
    default:
        return MTLSamplerMinMagFilterLinear;
    }
}

inline static MTLSamplerAddressMode ToMetalAddressMode(AddressMode mode) {
    switch (mode) {
    case AddressMode::Wrap:
        return MTLSamplerAddressModeRepeat;
    case AddressMode::Clamp:
        return MTLSamplerAddressModeClampToEdge;
    default:
        return MTLSamplerAddressModeClampToEdge;
    }
}

inline static MTLSamplerDescriptor* FillMetalSamplerDesc(const SamplerDesc& src) {
    MTLSamplerDescriptor* desc = [MTLSamplerDescriptor new];

    desc.minFilter = ToMetalFilter(src.min_filter);
    desc.magFilter = ToMetalFilter(src.mag_filter);
    desc.mipFilter = src.min_filter == MinFilter::LinearMipmapLinear ? MTLSamplerMipFilterLinear : MTLSamplerMipFilterNearest;

    desc.sAddressMode = ToMetalAddressMode(src.address_u);
    desc.tAddressMode = ToMetalAddressMode(src.address_v);
    desc.rAddressMode = ToMetalAddressMode(src.address_w);

    desc.maxAnisotropy = std::max(1u, src.max_anisotropy);
    desc.compareFunction = ToCompare(src.comparison_func);
    desc.lodMinClamp = src.min_lod;
    desc.lodMaxClamp = src.max_lod;

    return desc;
}

inline static bool HasStencil(PixelFormat format) {
    switch (format) {
    case PixelFormat::D24_UNORM_S8_UINT:
    case PixelFormat::D32_FLOAT_S8X24_UINT:
        return true;

    default:
        return false;
    }
}

} // namespace cave::render
