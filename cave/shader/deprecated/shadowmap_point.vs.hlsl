/// File: shadowmap_point.vs.hlsl
#include "cbuffer.slang.h"
#include "hlsl/input_output.hlsl"

VS_OUTPUT_POSITION vs_main(VS_INPUT_MESH input,
                        uint instance_id : SV_InstanceID) {
    float4x4 world_matrix;
    switch (c_mesh_flag) {
        case MESH_HAS_BONE: {
            float4x4 bone_matrix = c_bones[input.boneIndex.x] * input.boneWeight.x;
            bone_matrix += c_bones[input.boneIndex.y] * input.boneWeight.y;
            bone_matrix += c_bones[input.boneIndex.z] * input.boneWeight.z;
            bone_matrix += c_bones[input.boneIndex.w] * input.boneWeight.w;
            world_matrix = mul(c_world_matrix, bone_matrix);
        } break;
        case MESH_HAS_INSTANCE: {
            world_matrix = c_bones[instance_id];
        } break;
        default: {
            world_matrix = c_world_matrix;
        } break;
    }

    float4 position = mul(world_matrix, float4(input.position, 1.0));
    VS_OUTPUT_POSITION output;
    output.world_position = position.xyz;
    output.position = mul(c_pointLightMatrix, position);
    return output;
}
