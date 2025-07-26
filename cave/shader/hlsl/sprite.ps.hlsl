/// File: sprite.ps.hlsl
#include "hlsl/input_output.hlsl"
#include "cbuffer.hlsl.h"
#include "sampler.hlsl.h"

Texture2D t_Sprite : register(t0);

float4 main(VS_OUTPUT_UV input)
    : SV_TARGET {

    float4 sprite_color = t_Sprite.Sample(s_pointClampSampler, input.uv);

    float4 color = sprite_color * c_tint_color;
    if (color.a < 0.01f) {
        discard;
    }

    return float4(color.rgb, 1.0f);
}
