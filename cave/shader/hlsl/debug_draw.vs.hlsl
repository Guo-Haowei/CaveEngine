/// File: debug_draw.vs.hlsl
#include "cbuffer.hlsl.h"

struct VS_INPUT_COLOR {
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

struct VS_OUTPUT_COLOR {
    float4 position : SV_POSITION;
    float2 uv: TEXCOORD;
    float4 color : COLOR;
};

VS_OUTPUT_COLOR main(VS_INPUT_COLOR input) {
    float4 position = float4(input.position, 1.0);
    position = mul(c_camView, position);
    position = mul(c_camProj, position);

    VS_OUTPUT_COLOR output;
    output.position = position;
    output.uv = input.uv;
    output.color = input.color;
    return output;
}