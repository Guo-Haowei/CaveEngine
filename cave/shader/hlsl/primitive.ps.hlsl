/// File: primitive.ps.hlsl
#include "sampler.hlsl.h"
#include "shader_resource_defines.hlsl.h"
#include "hlsl/input_output.hlsl"

Texture2D t_Sprite : register(t0);

float4 main(VS_OUTPUT_COLOR input)
    : SV_TARGET {

    float4 color = t_Sprite.Sample(s_pointWrapSampler, input.uv);
    color *= input.color;

    if (color.a < 0.01f) {
        discard;
    }

    return color;
}
