/// File: primitive.slang
#include "../cbuffer.hlsl.h"

// -----------------------------------------------------------------------------
// Input / Output Structures
// -----------------------------------------------------------------------------

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

#if defined(__TARGET_GLSL__)
Sampler2D    u_Texture0;
#else
Texture2D    u_Texture0 : register(t0);
SamplerState s_pointWrapSampler : register(s1);
#endif

// -----------------------------------------------------------------------------
// Vertex Shader Entry Point
// -----------------------------------------------------------------------------

[shader("vertex")]
VSOutput vs_main(VSInput input)
{
    VSOutput output;

    float4 position = float4(input.position, 1.0);
    position = mul(c_camView, position);
    position = mul(c_camProj, position);

    output.position = position;
    output.uv       = input.uv;
    output.color    = input.color;

    return output;
}

[shader("fragment")]
float4 ps_main(VSOutput input) : SV_Target
{

#if defined(__TARGET_GLSL__)
    float4 color = u_Texture0.Sample(input.uv);
#else
    float4 color = u_Texture0.Sample(s_pointWrapSampler, input.uv);
#endif

    color *= input.color;

    if (color.a < 0.01f) {
        discard;
    }

    return color;
}
