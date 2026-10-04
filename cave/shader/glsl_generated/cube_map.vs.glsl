/// File: cube_map.vs.glsl
#version 410
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif

layout(binding = 0, std140) uniform type_PerBatchConstantBuffer
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

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 0) out vec3 out_var_POSITION;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    gl_Position = vec4(in_var_POSITION, 1.0) * spvWorkaroundRowMajor(PerBatchConstantBuffer.c_cubeProjectionViewMatrix);
    out_var_POSITION = in_var_POSITION;
}

