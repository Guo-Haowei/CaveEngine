/// File: skybox.ps.glsl
#version 410
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif

uniform samplerCube SPIRV_Cross_Combinedt_Skyboxs_cubemapClampSampler;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 0) out vec4 out_var_SV_TARGET;

void main()
{
    out_var_SV_TARGET = vec4(textureLod(SPIRV_Cross_Combinedt_Skyboxs_cubemapClampSampler, in_var_POSITION, 0.0).xyz, 1.0);
}

