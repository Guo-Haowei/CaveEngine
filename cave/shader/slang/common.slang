#pragma once
#if defined(__TARGET_GLSL__)

#define TEXTURE_2D(SAM, DUMMY, UV) SAM.Sample(UV)

Sampler2D    u_Texture0;
Sampler2D    u_Texture1;
Sampler2D    u_Texture2;

#elif defined(__TARGET_HLSL__)

#define TEXTURE_2D(TEX, SAM, UV) TEX.Sample(SAM, UV)

Texture2D    u_Texture0 : register(t0);
Texture2D    u_Texture1 : register(t1);
Texture2D    u_Texture2 : register(t2);

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
