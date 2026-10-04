/// File: shadow.vs.glsl
#version 450
#ifdef GL_ARB_shader_draw_parameters
#extension GL_ARB_shader_draw_parameters : enable
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

layout(binding = 1, std140) uniform type_PerPassConstantBuffer
{
    layout(row_major) mat4 c_viewMatrix;
    layout(row_major) mat4 c_projectionMatrix;
    layout(row_major) mat4 _per_pass_padding_0;
    layout(row_major) mat4 _per_pass_padding_1;
} PerPassConstantBuffer;

layout(binding = 3, std140) uniform type_BoneConstantBuffer
{
    layout(row_major) mat4 c_bones[128];
} BoneConstantBuffer;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 4) in ivec4 in_var_BONEINDEX;
layout(location = 5) in vec4 in_var_BONEWEIGHT;
#ifdef GL_ARB_shader_draw_parameters
#define SPIRV_Cross_BaseInstance gl_BaseInstanceARB
#else
uniform int SPIRV_Cross_BaseInstance;
#endif

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    mat4 _106;
    switch (PerBatchConstantBuffer.c_meshFlag)
    {
        case 1:
        {
            mat4 _54 = spvWorkaroundRowMajor(BoneConstantBuffer.c_bones[in_var_BONEINDEX.x]) * in_var_BONEWEIGHT.x;
            mat4 _59 = spvWorkaroundRowMajor(BoneConstantBuffer.c_bones[in_var_BONEINDEX.y]) * in_var_BONEWEIGHT.y;
            mat4 _76 = spvWorkaroundRowMajor(BoneConstantBuffer.c_bones[in_var_BONEINDEX.z]) * in_var_BONEWEIGHT.z;
            mat4 _89 = spvWorkaroundRowMajor(BoneConstantBuffer.c_bones[in_var_BONEINDEX.w]) * in_var_BONEWEIGHT.w;
            _106 = mat4(((_54[0] + _59[0]) + _76[0]) + _89[0], ((_54[1] + _59[1]) + _76[1]) + _89[1], ((_54[2] + _59[2]) + _76[2]) + _89[2], ((_54[3] + _59[3]) + _76[3]) + _89[3]) * spvWorkaroundRowMajor(PerBatchConstantBuffer.c_worldMatrix);
            break;
        }
        case 2:
        {
            _106 = spvWorkaroundRowMajor(BoneConstantBuffer.c_bones[uint((gl_InstanceID + SPIRV_Cross_BaseInstance))]);
            break;
        }
        default:
        {
            _106 = spvWorkaroundRowMajor(PerBatchConstantBuffer.c_worldMatrix);
            break;
        }
    }
    gl_Position = ((vec4(in_var_POSITION, 1.0) * _106) * spvWorkaroundRowMajor(PerPassConstantBuffer.c_viewMatrix)) * spvWorkaroundRowMajor(PerPassConstantBuffer.c_projectionMatrix);
}

