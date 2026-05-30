/// File: ui_overlay.ps.hlsl
#include "hlsl/input_output.hlsl"

float4 main(VS_OUTPUT_UI input)
    : SV_TARGET {
    return input.color;
}
