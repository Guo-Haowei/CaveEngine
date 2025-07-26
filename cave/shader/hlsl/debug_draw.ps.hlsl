/// File: debug_draw.ps.hlsl

struct VS_OUTPUT_COLOR {
    float4 position : SV_POSITION;
    float2 uv: TEXCOORD;
    float4 color : COLOR;
};

float4 main(VS_OUTPUT_COLOR input)
    : SV_TARGET {
    return input.color;
}
