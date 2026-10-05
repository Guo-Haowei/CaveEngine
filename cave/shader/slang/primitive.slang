/// File: primitive.slang
#include "../cbuffer.hlsl.h"
#include "../hlsl/input_output.hlsl"

#if defined(__TARGET_GLSL__)
Sampler2D    u_Texture0;
#else
Texture2D    u_Texture0 : register(t0);
SamplerState s_pointWrapSampler : register(s1);
#endif

[shader("vertex")]
VS_OUTPUT_COLOR vs_main(VS_INPUT_COLOR input)
{
    VS_OUTPUT_COLOR output;

    float4 position = float4(input.position, 1.0);
    position = mul(c_camView, position);
    position = mul(c_camProj, position);

    output.position = position;
    output.uv       = input.uv;
    output.color    = input.color;

    return output;
}

[shader("fragment")]
float4 ps_main(VS_OUTPUT_COLOR input) : SV_Target
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
