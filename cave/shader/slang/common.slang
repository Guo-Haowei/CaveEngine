#pragma once

#if defined(__TARGET_GLSL__)
#define TEXTURE_2D(NAME, BINDING) [vk::binding(BINDING, 0)] Sampler2D NAME
#define TEXTURE_CUBE(NAME, BINDING) [vk::binding(BINDING, 0)] SamplerCube NAME

#define SAMPLE_TEXTURE_2D(SAM, DUMMY, UV) SAM.Sample(UV)
#define SAMPLE_TEXTURE_LEVEL(SAM, DUMMY, UVW, MIP) SAM.SampleLevel(UVW, MIP)

#elif defined(__TARGET_HLSL__)

#define TEXTURE_2D(NAME, BINDING) Texture2D NAME : register(t##BINDING)
#define TEXTURE_CUBE(NAME, BINDING) TextureCube NAME : register(t##BINDING)

#define SAMPLE_TEXTURE_2D(TEX, SAM, UV) TEX.Sample(SAM, UV)
#define SAMPLE_TEXTURE_LEVEL(TEX, SAM, UVW, MIP) TEX.SampleLevel(SAM, UVW, MIP)

#define SAMPLER_STATE(REG, NAME, DESC) SamplerState NAME : register(s##REG);
#include "../sampler.slang.h"
#undef SAMPLER_STATE

#else
#error "Language NOT supported!"
#endif

float3 NdcToViewPos(float2 uv, float raw_depth, float4x4 inv_cam_proj) {
    float2 ndc = 2.0f * uv - 1.0f;

#if defined(__TARGET_GLSL__) && !defined(__TARGET_SPIRV__)
    float clip_depth = 2.0f * raw_depth - 1.0f;
#else
    ndc.y = -ndc.y;

    float clip_depth = raw_depth;
#endif

    float4 screenPos = float4(ndc.x, ndc.y, clip_depth, 1.0f);
    float4 viewPosH = mul(inv_cam_proj, screenPos);
    
    return viewPosH.xyz / viewPosH.w;
}