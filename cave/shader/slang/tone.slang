/// File: post_process.slang
#include "../cbuffer.slang.h"
#include "input_output.slang"

#include "common.slang"

TEXTURE_2D(u_Texture0, 0);

#define t_TextureLighting        u_Texture0

[shader("fragment")]
float4 ps_main(VS_OUTPUT_UV input) : SV_TARGET {
    float2 uv = input.uv;

    float3 hdr_color = SAMPLE_2D(PASS(t_TextureLighting), s_linearClampSampler, uv).rgb;

    const float exposure = 1.0f;
    const float gamma = 2.2f;

    float3 color = hdr_color * exposure;
    color = color / (color + float3(1.0f, 1.0f, 1.0f));
    color = pow(color, 1.0f / float3(gamma, gamma, gamma));

    return float4(color, 1.0);
}
