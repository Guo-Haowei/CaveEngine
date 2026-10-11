#pragma once

#if defined(__TARGET_D3D12__)
Texture2D   g_Textures2D[]   : register(t0, space1);
TextureCube g_TexturesCube[] : register(t0, space2);
#endif

// -----------------------------------------------------------------------------
// Texture Declarations
// -----------------------------------------------------------------------------
#if defined(__TARGET_OPENGL__)
    #define TEXTURE_2D(NAME, BINDING)   [vk::binding(BINDING, 0)] Sampler2D NAME;
    #define TEXTURE_CUBE(NAME, BINDING)  [vk::binding(BINDING, 0)] SamplerCube NAME;
#elif defined(__TARGET_D3D11__) || defined(__TARGET_METAL__)
    #define TEXTURE_2D(NAME, BINDING)   Texture2D NAME : register(t##BINDING);
    #define TEXTURE_CUBE(NAME, BINDING)  TextureCube NAME : register(t##BINDING);
#elif defined(__TARGET_D3D12__)
    #define TEXTURE_2D(NAME, BINDING)   static const uint NAME = BINDING;
    #define TEXTURE_CUBE(NAME, BINDING)  static const uint NAME = BINDING;
#else
    #error "Language NOT supported!"
#endif

// -----------------------------------------------------------------------------
// Texture Accessor
// -----------------------------------------------------------------------------
#if defined(__TARGET_D3D12__)
    #define TEX_2D(NAME) (g_Textures2D[NonUniformResourceIndex(c_pass_textures[NAME])])
    #define TEX_CUBE(NAME) (g_TexturesCube[NonUniformResourceIndex(c_pass_textures[NAME])])
#else
    #define TEX_2D(NAME) NAME
    #define TEX_CUBE(NAME) NAME
#endif

// -----------------------------------------------------------------------------
// Texture Sampling
// -----------------------------------------------------------------------------
#if defined(__TARGET_OPENGL__)
    #define SAMPLE_2D(SAM, DUMMY, UV)               SAM.Sample(UV)
    #define SAMPLE_CUBE(SAM, DUMMY, DIR)            SAM.Sample(DIR)
    #define SAMPLE_LEVEL(SAM, DUMMY, UVW, MIP)      SAM.SampleLevel(UVW, MIP)
    #define SAMPLE_CUBE_LEVEL(SAM, DUMMY, UVW, MIP) SAM.SampleLevel(UVW, MIP)
#else
    #define SAMPLE_2D(TEX, SAM, UV)                 TEX_2D(TEX).Sample(SAM, UV)
    #define SAMPLE_CUBE(TEX, SAM, DIR)              TEX_CUBE(TEX).Sample(SAM, DIR)
    #define SAMPLE_LEVEL(TEX, SAM, UVW, MIP)        TEX_2D(TEX).SampleLevel(SAM, UVW, MIP)
    #define SAMPLE_CUBE_LEVEL(TEX, SAM, DIR, MIP)   TEX_CUBE(TEX).SampleLevel(SAM, DIR, MIP)

    #define SAMPLER_STATE(REG, NAME, DESC) SamplerState NAME : register(s##REG);
    #include "../sampler.slang.h"
    #undef SAMPLER_STATE
#endif

float3 NdcToViewPos(float2 uv, float raw_depth, float4x4 inv_cam_proj) {
    float2 ndc = 2.0f * uv - 1.0f;

#if defined(__TARGET_OPENGL__) && !defined(__TARGET_SPIRV__)
    float clip_depth = 2.0f * raw_depth - 1.0f;
#else
    ndc.y = -ndc.y;

    float clip_depth = raw_depth;
#endif

    float4 screenPos = float4(ndc.x, ndc.y, clip_depth, 1.0f);
    float4 viewPosH = mul(inv_cam_proj, screenPos);

    return viewPosH.xyz / viewPosH.w;
}

float4 Mul(float4x4 matrix, float4 vector) {
#if defined(__TARGET_METAL__)
    return mul(vector, matrix);
#else
    return mul(matrix, vector);
#endif
}