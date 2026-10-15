// =============================================================================
// File: cave/render/rhi/Sampler.h
// =============================================================================
#pragma once
#include "cave/render/rhi/Defines.h"

namespace cave::render {

struct SamplerDesc {
    constexpr SamplerDesc(MinFilter min_filter_ = MinFilter::Point,
                          MagFilter mag_filter_ = MagFilter::Point,
                          AddressMode address_mode = AddressMode::Wrap,
                          StaticBorderColor border_color = StaticBorderColor::TransparentBlack,
                          float mip_lod_bias_ = 0.0f,
                          uint32_t max_anisotropy_ = 1.0f,
                          ComparisonFunc comparison_func_ = ComparisonFunc::LESS_EQUAL,
                          float min_lod_ = 0.0f,
                          float max_lod_ = 3.402823466e+38f) {
        min_filter = min_filter_;
        mag_filter = mag_filter_;
        address_u = address_mode;
        address_v = address_mode;
        address_w = address_mode;
        static_border_color = border_color;
        mip_lod_bias = mip_lod_bias_;
        max_anisotropy = max_anisotropy_;
        comparison_func = comparison_func_;
        min_lod = min_lod_;
        max_lod = max_lod_;

        switch (border_color) {
            case StaticBorderColor::OpaqueBlack:
                border[0] = 0.0f;
                border[1] = 0.0f;
                border[2] = 0.0f;
                border[3] = 1.0f;
                break;
            case StaticBorderColor::OpaqueWhite:
                border[0] = 1.0f;
                border[1] = 1.0f;
                border[2] = 1.0f;
                border[3] = 1.0f;
                break;
            case StaticBorderColor::TransparentBlack:
                [[fallthrough]];
            default:
                border[0] = 0.0f;
                border[1] = 0.0f;
                border[2] = 0.0f;
                border[3] = 0.0f;
                break;
        }
    }

    // @TODO: mipmap filter
    MinFilter min_filter;
    MagFilter mag_filter;

    AddressMode address_u;
    AddressMode address_v;
    AddressMode address_w;

    StaticBorderColor static_border_color;

    float mip_lod_bias;
    uint32_t max_anisotropy;
    ComparisonFunc comparison_func;
    float border[4];
    float min_lod;
    float max_lod;
};

static constexpr inline SamplerDesc PointClampSampler() {
    SamplerDesc desc(MinFilter::Point, MagFilter::Point, AddressMode::Clamp);
    return desc;
}

static constexpr inline SamplerDesc PointWrapSampler() {
    SamplerDesc desc(MinFilter::Point, MagFilter::Point, AddressMode::Wrap);
    return desc;
}

static constexpr inline SamplerDesc LinearClampSampler() {
    SamplerDesc desc(MinFilter::Linear, MagFilter::Linear, AddressMode::Clamp);
    return desc;
}

static constexpr inline SamplerDesc LinearWrapSampler() {
    SamplerDesc desc(MinFilter::Linear, MagFilter::Linear, AddressMode::Wrap);
    return desc;
}

SamplerDesc CubemapNoMipSampler();
SamplerDesc CubemapSampler();
SamplerDesc CubemapLodSampler();
SamplerDesc ShadowMapSampler();

// @TODO: refactor
inline SamplerDesc bloom_downsample() {
    SamplerDesc desc{};
    desc.min_filter = MinFilter::Linear;
    desc.mag_filter = MagFilter::Linear;
    desc.address_u = desc.address_v = desc.address_w = AddressMode::Border;
    desc.border[0] = 0.0;
    desc.border[1] = 0.0;
    desc.border[2] = 0.0;
    desc.border[3] = 1.0f;
    return desc;
}

inline SamplerDesc env_cube_map_sampler_mip() {
    SamplerDesc desc{};
    desc.min_filter = MinFilter::LinearMipmapLinear;
    desc.mag_filter = MagFilter::Linear;
    desc.address_u = desc.address_v = desc.address_w = AddressMode::Clamp;
    return desc;
}

}  // namespace cave::render
