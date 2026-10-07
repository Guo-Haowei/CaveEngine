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

float3 NdcToViewPos(float2 uv, float depth, float4x4 inv_cam_proj) {
    float2 ndc = 2.0f * uv - 1.0f;

    // 2. Convert raw G-Buffer depth to NDC Z-depth
#if defined(__TARGET_GLSL__) && !defined(__TARGET_SPIRV__)
    float zClip = 2.0f * depth - 1.0f;
    ndc.y = -ndc.y;
#else
    float zClip = depth;
#endif

    // 3. Unproject to View Space
    float4 clipPos = float4(ndc.x, ndc.y, zClip, 1.0f);
    float4 viewPosH = mul(inv_cam_proj, clipPos);
    
    return viewPosH.xyz / viewPosH.w;
}