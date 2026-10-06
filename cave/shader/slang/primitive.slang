/// File: primitive.slang
#include "../cbuffer.hlsl.h"
#include "input_output.slang"
#include "common.slang"

TEXTURE_2D(u_Texture0, 0); // t_Sprite

[shader("vertex")]
VS_OUTPUT_COLOR vs_main(VS_INPUT_COLOR input)
{
    VS_OUTPUT_COLOR output;

    float4 position = float4(input.position, 1.0);
    position = mul(c_camView, position);
    position = mul(c_camProj, position);

    output.position = position;
    output.uv       = input.uv;
    output.color    = input.color;

    return output;
}

[shader("fragment")]
float4 ps_main(VS_OUTPUT_COLOR input) : SV_Target
{
    float4 color = SAMPLE_TEXTURE_2D(u_Texture0, s_pointWrapSampler, input.uv);

    color *= input.color;

    if (color.a < 0.01f) {
        discard;
    }

    return color;
}
