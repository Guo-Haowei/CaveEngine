/// File: to_cube_map.slang
#include "../cbuffer.hlsl.h"
#include "../hlsl/input_output.hlsl"
#include "common.slang"

// Texture2D t_SkyboxHdr : register(t0);

float2 sample_spherical_map(float3 v) {
    const float2 inv_atan = float2(0.1591, 0.3183);
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= inv_atan;
    uv += 0.5;
    uv.y = 1.0 - uv.y;
    return uv;
}

[shader("fragment")]
float4 ps_main(VS_OUTPUT_POSITION input)
    : SV_TARGET {
    float2 uv = sample_spherical_map(normalize(input.world_position));

    float4 color = TEXTURE_2D(u_Texture0, s_linearClampSampler, uv);
    return float4(color.rgb, 1.0f);
}
