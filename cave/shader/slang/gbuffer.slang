/// File: gbuffer.slang
#include "../cbuffer.hlsl.h"
#include "input_output.slang"

#include "common.slang"

TEXTURE_2D(BaseColorMap, t30);
TEXTURE_2D(NormalMap, t31);
TEXTURE_2D(MaterialMap, t32);

struct ps_output {
    float4 base_color : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 out_emissive_roughness_metallic : SV_TARGET2;
};

ps_output ps_main(VS_OUTPUT_MESH input) {
    float4 color = c_baseColor;

    if (c_hasBaseColorMap != 0) {
        color = SAMPLE_TEXTURE_2D(BaseColorMap, s_linearMipWrapSampler, input.uv);
    }

    if (color.a <= 0.0) {
        discard;
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

    ps_output output;
    output.base_color = color;
#if 0
    output.base_color = float4(0.5f * N + 0.5f, 1.0f);
#endif

    output.normal.xyz = 0.5f * N + 0.5f;
    output.normal.a = 1.0f;

    output.out_emissive_roughness_metallic.r = c_emissivePower;
    output.out_emissive_roughness_metallic.g = roughness;
    output.out_emissive_roughness_metallic.b = metallic;
    output.out_emissive_roughness_metallic.a = 1.0f;
    return output;
}
