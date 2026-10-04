/// File: skybox.ps.glsl
uniform samplerCube SPIRV_Cross_Combinedt_Skyboxs_cubemapClampSampler;

layout(location = 0) in vec3 pass_position;
layout(location = 0) out vec4 out_color;

void main()
{
    out_color = vec4(textureLod(SPIRV_Cross_Combinedt_Skyboxs_cubemapClampSampler, pass_position, 0.0).xyz, 1.0);
}

