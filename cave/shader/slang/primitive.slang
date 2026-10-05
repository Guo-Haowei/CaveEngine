// -----------------------------------------------------------------------------
// Constant Buffers
// -----------------------------------------------------------------------------

cbuffer PerBatchConstantBuffer : register(b0)
{
    float4x4 c_worldMatrix;

    // Reuse per batch buffer for bloom
    float4 _dummy;

    float2 _per_batch_padding_0;
    float  c_envPassRoughness; // For environment map
    int    c_meshFlag;

    float4 c_tint_color;
    float4 c_uv_rect;

    float4x4 c_cubeProjectionViewMatrix;
    float4x4 _per_batch_padding_5;
};

cbuffer PerPassConstantBuffer : register(b1)
{
    float4x4 c_camView;
    float4x4 c_camProj;

    float4x4 _per_pass_padding_0;
    float4x4 _per_pass_padding_1;
};

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

Texture2D    t_Sprite : register(t0);
SamplerState s_pointWrapSampler : register(s1);

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

    float4 color = t_Sprite.Sample(s_pointWrapSampler, input.uv);
    color *= input.color;

    if (color.a < 0.01f) {
        discard;
    }

    return color;
}
