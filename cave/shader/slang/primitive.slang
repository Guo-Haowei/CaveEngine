#define CAVE_CBUFFER(...)
#include "../cbuffer.hlsl.h"

cbuffer PerPassConPerFrameConstantBuffer : register(b5)
{
    Light c_lights[MAX_LIGHT_COUNT];

    float4 c_ssaoKernel[SSAO_KERNEL_SIZE];
    //-----------------------------------------
    float4x4 c_camProj;
    float4x4 c_camView;
    float4x4 c_invCamProj;
    float4x4 c_invCamView;

    float4 _per_frame_padding_2;
    float4 _per_frame_padding_3;
    float4 _per_frame_padding_4;
    float3 c_sunPosition;
    int c_iblEnabled;
    //-----------------------------------------
    float4 c_ambientColor;  // 16

    int c_lightCount;
    int c_enableBloom;
    int c_debugCsm;
    float c_bloomThreshold;  // 16

    int c_debugVoxelId;
    int c_ssaoEnabled;
    int c_enableVxgi;
    float c_texelSize;  // 16

    float2 c_screen_size;
    float c_ssaoKernalRadius;
    int c_ptObjectCount;
    //-----------------------------------------
    uint c_DiffuseIrradianceResidentHandle;
    uint c_PrefilteredResidentHandle;
    uint c_BrdfLutResidentHandle;
    int c_forceFieldsCount;  // 16

    float4 _c_SkyboxHdrResidentHandle;  // 16
    float4 _c_ShadowMapResidentHandle;

    float3 c_cameraPosition;
    float c_camera_fovy;  // 16
    //-----------------------------------------
    float3 c_voxelWorldCenter;
    float c_voxelWorldSizeHalf;  // 16

    float3 c_cameraForward;
    uint c_frame_index;  // 16

    float3 c_cameraRight;
    int c_scene_dirty;  // 16

    float3 c_cameraUp;
    float c_voxelSize;  // 16
    //-----------------------------------------

    ForceField c_forceFields[MAX_FORCE_FIELD_COUNT];
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

Texture2D    u_Texture0 : register(t0);
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

    float4 color = u_Texture0.Sample(s_pointWrapSampler, input.uv);
    color *= input.color;

    if (color.a < 0.01f) {
        discard;
    }

    return color;
}
