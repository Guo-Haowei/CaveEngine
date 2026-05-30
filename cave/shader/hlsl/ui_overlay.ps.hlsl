/// File: ui_overlay.ps.hlsl

struct VS_OUTPUT_COLOR {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main(VS_OUTPUT_COLOR input)
    : SV_TARGET {
    return input.color;
}
