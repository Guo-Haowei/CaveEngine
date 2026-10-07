/// File: lighting.ps.hlsl
#include "../cbuffer.hlsl.h"
#include "input_output.slang"

#include "common.slang"

#include "../pbr.hlsl.h"

TEXTURE_2D(u_Texture0, 0);
TEXTURE_2D(u_Texture1, 1);
TEXTURE_2D(u_Texture2, 2);
TEXTURE_2D(u_Texture3, 3);
TEXTURE_2D(u_Texture4, 4);
TEXTURE_2D(u_Texture5, 5);
TEXTURE_CUBE(u_Texture6, 6);
TEXTURE_CUBE(u_Texture7, 7);
TEXTURE_2D(u_Texture8, 8);

#define t_GbufferBaseColorMap u_Texture0
#define t_GbufferNormalMap    u_Texture1
#define t_GbufferMaterialMap  u_Texture2
#define t_GbufferDepth        u_Texture3
#define t_SsaoMap             u_Texture4
#define t_ShadowMap           u_Texture5
#define t_DiffuseIrradiance   u_Texture6
#define t_Prefiltered         u_Texture7
#define t_BrdfLut             u_Texture8
#define t_LTC1                u_Texture9
#define t_LTC2                u_Texture10
#define t_VoxelLighting       u_Texture11

TEXTURE_2D(BaseColorMap, 30);
TEXTURE_2D(NormalMap, 31);
TEXTURE_2D(MaterialMap, 32);

#include "common_lighting.slang"

[shader("fragment")]
float4 ps_main(VS_OUTPUT_UV input) : SV_TARGET {
    const float2 uv = input.uv;
    const float4 emissive_roughness_metallic = SAMPLE_TEXTURE_2D(t_GbufferMaterialMap, s_linearMipWrapSampler, uv);
    clip(emissive_roughness_metallic.a - 0.01f);

    float3 base_color = SAMPLE_TEXTURE_2D(t_GbufferBaseColorMap, s_linearMipWrapSampler, uv).rgb;

    const float raw_depth = SAMPLE_TEXTURE_2D(t_GbufferDepth, s_pointClampSampler, uv).r;
    const float3 view_position = NdcToViewPos(uv, raw_depth, c_invCamProj);
    const float4 world_position = mul(c_invCamView, float4(view_position, 1.0f));

    float emissive = emissive_roughness_metallic.r;
    float roughness = emissive_roughness_metallic.g;
    float metallic = emissive_roughness_metallic.b;

    if (emissive > 0.0) {
        return float4(emissive * base_color, 1.0);
    }

    float3 N = SAMPLE_TEXTURE_2D(t_GbufferNormalMap, s_linearMipWrapSampler, uv).rgb;
    N = 2.0f * N - 1.0f;

    float3 color = compute_lighting(t_ShadowMap,
                                    base_color,
                                    world_position.xyz,
                                    N,
                                    metallic,
                                    roughness,
                                    emissive);
    if (c_ssaoEnabled != 0) {
        float ao = SAMPLE_TEXTURE_2D(t_SsaoMap, s_pointClampSampler, uv).r;
        color *= ao;
    }

    return float4(color, 1.0f);
}
