#ifndef SAMPLER_STATE
#define SAMPLER_STATE(...)
#endif

SAMPLER_STATE(0, s_pointClampSampler, PointClampSampler())
SAMPLER_STATE(1, s_pointWrapSampler, PointWrapSampler())
SAMPLER_STATE(2, s_linearMipWrapSampler, LinearWrapSampler())
SAMPLER_STATE(3, s_shadowSampler, ShadowMapSampler())
SAMPLER_STATE(4, s_linearClampSampler, LinearClampSampler())
SAMPLER_STATE(5, s_cubemapClampSampler, CubemapSampler())
SAMPLER_STATE(6, s_cubemapClampLodSampler, CubemapLodSampler())