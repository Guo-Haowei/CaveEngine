/// File: shadowmap_point.vs.glsl
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

layout(binding = 3, std140) uniform type_BoneConstantBuffer
{
    layout(row_major) mat4 c_bones[128];
} BoneConstantBuffer;

layout(binding = 4, std140) uniform type_PointShadowConstantBuffer
{
    layout(row_major) mat4 c_pointLightMatrix;
    vec3 c_pointLightPosition;
    float c_pointLightFar;
    vec4 _point_shadow_padding_0;
    vec4 _point_shadow_padding_1;
    vec4 _point_shadow_padding_2;
    layout(row_major) mat4 _point_shadow_padding_3;
    layout(row_major) mat4 _point_shadow_padding_4;
} PointShadowConstantBuffer;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 4) in ivec4 in_var_BONEINDEX;
layout(location = 5) in vec4 in_var_BONEWEIGHT;
#ifdef GL_ARB_shader_draw_parameters
#define SPIRV_Cross_BaseInstance gl_BaseInstanceARB
#else
uniform int SPIRV_Cross_BaseInstance;
#endif
layout(location = 0) out vec3 out_var_POSITION;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    mat4 _107;
    switch (PerBatchConstantBuffer.c_meshFlag)
    {
        case 1:
        {
            mat4 _55 = spvWorkaroundRowMajor(BoneConstantBuffer.c_bones[in_var_BONEINDEX.x]) * in_var_BONEWEIGHT.x;
            mat4 _60 = spvWorkaroundRowMajor(BoneConstantBuffer.c_bones[in_var_BONEINDEX.y]) * in_var_BONEWEIGHT.y;
            mat4 _77 = spvWorkaroundRowMajor(BoneConstantBuffer.c_bones[in_var_BONEINDEX.z]) * in_var_BONEWEIGHT.z;
            mat4 _90 = spvWorkaroundRowMajor(BoneConstantBuffer.c_bones[in_var_BONEINDEX.w]) * in_var_BONEWEIGHT.w;
            _107 = mat4(((_55[0] + _60[0]) + _77[0]) + _90[0], ((_55[1] + _60[1]) + _77[1]) + _90[1], ((_55[2] + _60[2]) + _77[2]) + _90[2], ((_55[3] + _60[3]) + _77[3]) + _90[3]) * spvWorkaroundRowMajor(PerBatchConstantBuffer.c_worldMatrix);
            break;
        }
        case 2:
        {
            _107 = spvWorkaroundRowMajor(BoneConstantBuffer.c_bones[uint((gl_InstanceID + SPIRV_Cross_BaseInstance))]);
            break;
        }
        default:
        {
            _107 = spvWorkaroundRowMajor(PerBatchConstantBuffer.c_worldMatrix);
            break;
        }
    }
    vec4 _112 = vec4(in_var_POSITION, 1.0) * _107;
    gl_Position = _112 * spvWorkaroundRowMajor(PointShadowConstantBuffer.c_pointLightMatrix);
    out_var_POSITION = _112.xyz;
}

