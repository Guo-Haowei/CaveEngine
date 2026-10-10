/// File: cube_map.slang
#include "../cbuffer.slang.h"
#include "input_output.slang"

[shader("vertex")]
VS_OUTPUT_POSITION vs_main(VS_INPUT_POS input) {
    VS_OUTPUT_POSITION output;
    output.position = mul(c_cube_proj_view_matrix, float4(input.position, 1.0));
    output.world_position = input.position;
    return output;
}
