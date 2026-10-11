/// File: gbuffer.slang
#include "../cbuffer.slang.h"
#include "input_output.slang"

#include "common.slang"

#include "../shader_resource_defines.slang.h"

struct ps_output {
    float4 base_color : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 out_emissive_roughness_metallic : SV_TARGET2;
};

ps_output ps_main(VS_OUTPUT_MESH input) {
    float4 color = c_baseColor;

    if (c_has_base_color_map != 0) {
        color =
#if defined(__TARGET_D3D12__)
        g_Textures2D[c_base_color_map_index].Sample(s_linearMipWrapSampler, input.uv);
#else
        SAMPLE_2D(BaseColorMap, s_linearMipWrapSampler, input.uv);
#endif
    }

    if (color.a <= 0.0) {
        discard;
    }

    float metallic;
    float roughness;

    if (c_has_material_map != 0) {
        float3 value =
#if defined(__TARGET_D3D12__)
        g_Textures2D[c_material_map_index].Sample(s_linearMipWrapSampler, input.uv).rgb;
#else
        SAMPLE_2D(MaterialMap, s_linearMipWrapSampler, input.uv).rgb;
#endif
        metallic = value.b;
        roughness = value.g;
    } else {
        metallic = c_metallic;
        roughness = c_roughness;
    }

    float3 N;
    if (c_has_normal_map != 0) {
        float3x3 TBN;
        TBN[0] = normalize(input.T);
        TBN[1] = normalize(input.B);
        TBN[2] = normalize(input.normal);
        float3 value =
#if defined(__TARGET_D3D12__)
        g_Textures2D[c_normal_map_index].Sample(s_linearMipWrapSampler, input.uv).rgb;
#else
        SAMPLE_2D(NormalMap, s_linearMipWrapSampler, input.uv).rgb;
#endif
        N = normalize(mul((2.0f * value) - 1.0f, TBN));
    } else {
        N = normalize(input.normal);
    }

    ps_output output;
    output.base_color = color;

    output.normal = float4(0.5f * N + 0.5f, 1.0f);

    output.out_emissive_roughness_metallic.r = c_emissive_power;
    output.out_emissive_roughness_metallic.g = roughness;
    output.out_emissive_roughness_metallic.b = metallic;
    output.out_emissive_roughness_metallic.a = 1.0f;
    return output;
}
