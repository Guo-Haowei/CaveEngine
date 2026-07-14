/// File: ui_overlay.vs.hlsl
#include "cbuffer.hlsl.h"
#include "hlsl/input_output.hlsl"

VS_OUTPUT_COLOR main(VS_INPUT_COLOR input) {
    float2 pos2 = input.position.xy;
    pos2 = pos2 / c_screen_size * 2.0f - 1.0f;
    pos2.y = -pos2.y;
    float4 pos = float4(pos2, 0.0, 1.0);

    VS_OUTPUT_COLOR output;
    output.position = pos;
    output.uv = input.uv;
    output.color = input.color;
    return output;
}
