/// File: primitive.slang
#include "../cbuffer.slang.h"
#include "input_output.slang"
#include "common.slang"

TEXTURE_2D(u_Texture0, 0); // t_Sprite

[shader("vertex")]
VsPrimOutput vs_main(VsPrimInput input)
{
    VsPrimOutput output;

    float4 position = float4(input.position, 1.0);
    position = Mul(c_cam_view, position);
    position = Mul(c_cam_proj, position);

    output.position = position;
    output.uv       = input.uv;
    output.color    = input.color;
    output.tex_id   = input.tex_id;

    return output;
}

[shader("fragment")]
float4 ps_main(VsPrimOutput input) : SV_Target
{
#if defined(__TARGET_D3D12__)
    Texture2D<float4> tex = ResourceDescriptorHeap[input.tex_id.x];
    float4 color = tex.Sample(s_pointWrapSampler, input.uv);
#else
    float4 color = SAMPLE_2D(u_Texture0, s_pointWrapSampler, input.uv);
#endif

    color *= input.color;

    if (color.a < 0.01f) {
        discard;
    }

    return color;
}
