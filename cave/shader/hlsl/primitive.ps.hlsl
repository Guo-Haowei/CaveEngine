/// File: primitive.ps.hlsl
#include "sampler.hlsl.h"
#include "shader_resource_defines.hlsl.h"

Texture2D t_Sprite : register(t0);

struct VS_OUTPUT_COLOR {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

float4 main(VS_OUTPUT_COLOR input)
    : SV_TARGET {

    float4 color = t_Sprite.Sample(s_pointClampSampler, input.uv);
    color *= input.color;

    if (color.a < 0.01f) {
        discard;
    }

    return color;
}
