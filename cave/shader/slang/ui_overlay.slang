/// File: ui_overlay.slang
#include "../cbuffer.slang.h"
#include "input_output.slang"

[shader("vertex")]
VsPrimOutput vs_main(VsPrimInput input) {
    float2 pos2 = input.position.xy;
    pos2 = pos2 / c_screen_size.xy * 2.0f - 1.0f;
    pos2.y = -pos2.y;
    float4 pos = float4(pos2, 0.0, 1.0);

    VsPrimOutput output;
    output.position = pos;
    output.uv = input.uv;
    output.uv.y = 1.0f - output.uv.y;
    output.color = input.color;
    output.tex_id   = input.tex_id;

    return output;
}
