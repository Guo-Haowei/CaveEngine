#pragma once
#if defined(__TARGET_GLSL__)

#define TEXTURE_2D(NAME, BINDING) Sampler2D NAME
#define TEXTURE_CUBE(NAME, BINDING) SamplerCube NAME

#define SAMPLE_TEXTURE_2D(SAM, DUMMY, UV) SAM.Sample(UV)
#define SAMPLE_TEXTURE_LEVEL(SAM, DUMMY, UVW, MIP) SAM.SampleLevel(UVW, MIP)

#elif defined(__TARGET_HLSL__)

#define TEXTURE_2D(NAME, BINDING) Texture2D NAME : register(BINDING)
#define TEXTURE_CUBE(NAME, BINDING) TextureCube NAME : register(BINDING)

#define SAMPLE_TEXTURE_2D(TEX, SAM, UV) TEX.Sample(SAM, UV)
#define SAMPLE_TEXTURE_LEVEL(TEX, SAM, UVW, MIP) TEX.SampleLevel(SAM, UVW, MIP)

SamplerState s_pointClampSampler : register(s0);
SamplerState s_pointWrapSampler : register(s1);
SamplerState s_linearMipWrapSampler : register(s2);
SamplerState s_shadowSampler : register(s3);
SamplerState s_linearClampSampler : register(s4);
SamplerState s_cubemapClampSampler : register(s5);
SamplerState s_cubemapClampLodSampler : register(s6);

#else
#error "Language NOT supported!"
#endif
