#version 460

layout(binding = 0, std140) uniform SLANG_ParameterGroup_PerBatchConstantBuffer_std140
{
    layout(row_major) mat4 c_worldMatrix;
    vec4 _dummy;
    vec2 _per_batch_padding_0;
    float c_envPassRoughness;
    int c_meshFlag;
    vec4 c_tint_color;
    vec4 c_uv_rect;
    layout(row_major) mat4 c_cubeProjectionViewMatrix;
    layout(row_major) mat4 _per_batch_padding_5;
} PerBatchConstantBuffer;

layout(location = 0) in vec3 input_position;
layout(location = 0) out vec3 entryPointParam_vs_main_world_position;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    gl_Position = vec4(input_position, 1.0) * spvWorkaroundRowMajor(PerBatchConstantBuffer.c_cubeProjectionViewMatrix);
    entryPointParam_vs_main_world_position = input_position;
}

