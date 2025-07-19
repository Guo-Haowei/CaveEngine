/// File: sprite.ps.hlsl
#include "hlsl/input_output.hlsl"
#include "sampler.hlsl.h"

Texture2D t_Sprite : register(t0);

float4 main(VS_OUTPUT_UV input)
    : SV_TARGET
{

    float4 sprite_color = t_Sprite.Sample(s_pointClampSampler, input.uv);
    float4 tint_color = float4(1.0, 1.0, 1.0, 1.0);

#if 0
    return tint_color;
#else
    float4 color = sprite_color * tint_color;
    if (color.a < 0.01f)
    {
        discard;
    }

    return float4(color.rgb, 1.0f);
#endif
}
