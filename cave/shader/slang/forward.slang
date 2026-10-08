/// File: forward.ps.hlsl
#include "../cbuffer.hlsl.h"
#include "input_output.slang"

#include "common.slang"

TEXTURE_CUBE(u_Texture0, 0);
TEXTURE_2D(u_Texture1, 1);
TEXTURE_CUBE(u_Texture2, 2);
TEXTURE_CUBE(u_Texture3, 3);
TEXTURE_2D(u_Texture4, 4);
TEXTURE_2D(u_Texture5, 5);
TEXTURE_2D(u_Texture6, 6);

#define t_Skybox            u_Texture0
#define t_ShadowMap         u_Texture1
#define t_DiffuseIrradiance u_Texture2
#define t_Prefiltered       u_Texture3
#define t_BrdfLut           u_Texture4
#define t_LTC1              u_Texture5
#define t_LTC2              u_Texture6
// #define t_VoxelLighting     u_Texture6

#include "../shader_resource_defines.slang.h"
#include "common_lighting.slang"

[shader("fragment")]
float4 ps_main(VS_OUTPUT_MESH input)
    : SV_TARGET {
    const float2 texcoord = input.uv;
    const float4 world_position = mul(c_inv_cam_view, float4(input.world_position, 1.0f));

    float3 base_color = c_baseColor.rgb;
    float alpha = c_baseColor.a;

    if (c_hasBaseColorMap != 0) {
        float4 tmp = SAMPLE_TEXTURE_2D(BaseColorMap, s_linearMipWrapSampler, input.uv);
        base_color = tmp.rgb;
        alpha = tmp.a;
    }

    float metallic;
    float roughness;

    if (c_hasMaterialMap != 0) {
        float3 value = SAMPLE_TEXTURE_2D(MaterialMap, s_linearMipWrapSampler, input.uv).rgb;
        metallic = value.b;
        roughness = value.g;
    } else {
        metallic = c_metallic;
        roughness = c_roughness;
    }

    float3 N;
    if (c_hasNormalMap != 0) {
        float3x3 TBN;
        TBN[0] = normalize(input.T);
        TBN[1] = normalize(input.B);
        TBN[2] = normalize(input.normal);
        float3 value = SAMPLE_TEXTURE_2D(NormalMap, s_linearMipWrapSampler, input.uv).rgb;
        N = normalize(mul((2.0f * value) - 1.0f, TBN));
    } else {
        N = normalize(input.normal);
    }

    float3 color = compute_lighting(t_ShadowMap,
                                    base_color,
                                    world_position.xyz,
                                    N,
                                    metallic,
                                    roughness,
                                    c_emissivePower);
    return float4(color, alpha);
}
