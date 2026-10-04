/// File: shadowmap_point.ps.glsl
#version 410
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif

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

void main()
{
    gl_FragDepth = length(in_var_POSITION - PointShadowConstantBuffer.c_pointLightPosition) / PointShadowConstantBuffer.c_pointLightFar;
}

