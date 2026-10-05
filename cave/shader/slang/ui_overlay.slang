/// File: ui_overlay.slang
#include "../cbuffer.hlsl.h"

struct VSInput
{
    float3 position : POSITION;
    float2 uv       : TEXCOORD0;
    float4 color    : COLOR0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
    float4 color    : COLOR0;
};

[shader("vertex")]
VSOutput vs_main(VSInput input) {
    float2 pos2 = input.position.xy;
    pos2 = pos2 / c_screen_size * 2.0f - 1.0f;
    pos2.y = -pos2.y;
    float4 pos = float4(pos2, 0.0, 1.0);

    VSOutput output;
    output.position = pos;
    output.uv = input.uv;
    output.uv.y = 1.0f - output.uv.y;
    output.color = input.color;
    return output;
}
