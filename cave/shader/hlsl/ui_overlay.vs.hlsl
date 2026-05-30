/// File: ui_overlay.vs.hlsl
#include "cbuffer.hlsl.h"
#include "hlsl/input_output.hlsl"

VS_OUTPUT_UI main(VS_INPUT_UI input) {
    float2 pos2 = input.position;
    pos2 = pos2 / c_screen_size * 2.0f - 1.0f;
    pos2.y = -pos2.y;
    float4 pos = float4(pos2, 0.0, 1.0);

    VS_OUTPUT_UI output;
    output.position = pos;
    output.color = input.color;
    return output;
}
