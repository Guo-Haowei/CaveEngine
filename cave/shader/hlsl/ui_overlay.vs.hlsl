/// File: ui_overlay.vs.hlsl
#include "cbuffer.hlsl.h"

struct VS_INPUT_COLOR {
    float2 position : POSITION;
    float4 color : COLOR;
};

struct VS_OUTPUT_COLOR {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VS_OUTPUT_COLOR main(VS_INPUT_COLOR input) {
    float4 position = float4(input.position, 0.0, 1.0);

    VS_OUTPUT_COLOR output;
    output.position = position;
    output.color = input.color;
    return output;
}
