/// File: shadow.vs.glsl
#include "../cbuffer.hlsl.h"
#include "../vsinput.glsl.h"

void main() {
    mat4 world_matrix;
    switch (c_mesh_flag) {
        case MESH_HAS_BONE: {
            mat4 bone_matrix = c_bones[in_bone_id.x] * in_bone_weight.x;
            bone_matrix += c_bones[in_bone_id.y] * in_bone_weight.y;
            bone_matrix += c_bones[in_bone_id.z] * in_bone_weight.z;
            bone_matrix += c_bones[in_bone_id.w] * in_bone_weight.w;
            world_matrix = c_world_matrix * bone_matrix;
        } break;
        case MESH_HAS_INSTANCE: {
            world_matrix = c_bones[gl_InstanceID];
        } break;
        default: {
            world_matrix = c_world_matrix;
        } break;
    }

    // view space position
    vec4 position = c_viewMatrix * (world_matrix * vec4(in_position, 1.0));

    gl_Position = c_projectionMatrix * position;
}
