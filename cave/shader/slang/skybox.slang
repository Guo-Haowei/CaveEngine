/// File: skybox.ps.glsl
#include "../cbuffer.slang.h"
#include "input_output.slang"
#include "common.slang"

TEXTURE_CUBE(u_Texture0, 0); // t_Skybox

[shader("vertex")]
VS_OUTPUT_POSITION vs_main(VS_INPUT_MESH input) {
    float3x3 rotate = (float3x3)c_cam_view;
    float3 position3 = mul(rotate, input.position);
    float4 position = mul(c_cam_proj, float4(position3, 1.0));

    VS_OUTPUT_POSITION output;
    output.position = position.xyww;
    output.world_position = input.position;
    return output;
}

[shader("fragment")]
float4 ps_main(VS_OUTPUT_POSITION input) : SV_TARGET {
    float3 uvw = input.world_position;
#if defined(__TARGET_D3D11__) || defined(__TARGET_METAL__)
    uvw.y = -uvw.y;
#endif
    float3 color = SAMPLE_TEXTURE_LEVEL(u_Texture0, s_cubemapClampSampler, uvw, 0.0f).rgb;
    return float4(color, 1.0f);
}
