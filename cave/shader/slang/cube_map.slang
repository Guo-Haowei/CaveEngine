/// File: cube_map.slang
#include "../cbuffer.hlsl.h"
#include "../hlsl/input_output.hlsl"

VS_OUTPUT_POSITION vs_main(VS_INPUT_POS input) {
    VS_OUTPUT_POSITION output;
    output.position = mul(c_cubeProjectionViewMatrix, float4(input.position, 1.0));
    output.world_position = input.position;
    return output;
}
