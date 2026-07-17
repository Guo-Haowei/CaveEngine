#include "sampler.h"

#include "engine/private/render/render_graph/RenderGraphDefines.h"

namespace cave {

SamplerDesc CubemapNoMipSampler() {
    SamplerDesc desc(MinFilter::LINEAR,
                     MagFilter::LINEAR,
                     AddressMode::CLAMP,
                     StaticBorderColor::TRANSPARENT_BLACK,
                     0.0f,
                     1,
                     ComparisonFunc::ALWAYS);
    return desc;
}

SamplerDesc CubemapSampler() {
    SamplerDesc desc(MinFilter::LINEAR_MIPMAP_LINEAR,
                     MagFilter::LINEAR,
                     AddressMode::CLAMP,
                     StaticBorderColor::TRANSPARENT_BLACK,
                     0.0f,
                     1,
                     ComparisonFunc::ALWAYS);
    return desc;
}

SamplerDesc CubemapLodSampler() {
    SamplerDesc desc = CubemapSampler();
    desc.maxLod = kIBLMipChainMax - 1.0f;
    return desc;
}

SamplerDesc ShadowMapSampler() {
    SamplerDesc desc(MinFilter::LINEAR,
                     MagFilter::LINEAR,
                     AddressMode::BORDER, StaticBorderColor::OPAQUE_WHITE);
    return desc;
}

}  // namespace cave
