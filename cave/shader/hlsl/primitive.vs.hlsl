/// File: primitive.vs.hlsl
#include "cbuffer.hlsl.h"
#include "hlsl/input_output.hlsl"

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