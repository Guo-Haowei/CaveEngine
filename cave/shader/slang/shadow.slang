/// File: shadow.slang
#include "../cbuffer.hlsl.h"
#include "input_output.slang"

[shader("vertex")]
float4 vs_main(VS_INPUT_MESH input, uint instance_id : SV_InstanceID) : SV_POSITION {
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

    float4 position = float4(input.position, 1.0);
    position = mul(world_matrix, position);
    position = mul(c_viewMatrix, position);
    position = mul(c_projectionMatrix, position);

    return position;
}
