#include "cave/render/rhi/Sampler.h"

#include "engine/private/render/render_graph/RenderGraphDefines.h"

namespace cave::render {

SamplerDesc CubemapNoMipSampler() {
    SamplerDesc desc(MinFilter::Linear,
                     MagFilter::Linear,
                     AddressMode::Clamp,
                     StaticBorderColor::TransparentBlack,
                     0.0f,
                     1,
                     ComparisonFunc::ALWAYS);
    return desc;
}

SamplerDesc CubemapSampler() {
    SamplerDesc desc(MinFilter::LinearMipmapLinear,
                     MagFilter::Linear,
                     AddressMode::Clamp,
                     StaticBorderColor::TransparentBlack,
                     0.0f,
                     1,
                     ComparisonFunc::ALWAYS);
    return desc;
}

SamplerDesc CubemapLodSampler() {
    SamplerDesc desc = CubemapSampler();
    desc.max_lod = kIBLMipChainMax - 1.0f;
    return desc;
}

SamplerDesc ShadowMapSampler() {
    SamplerDesc desc(MinFilter::Linear,
                     MagFilter::Linear,
                     AddressMode::Border, StaticBorderColor::OpaqueWhite);
    return desc;
}

}  // namespace cave::render
