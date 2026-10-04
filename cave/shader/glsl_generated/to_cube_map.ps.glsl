/// File: to_cube_map.ps.glsl
#version 450

uniform sampler2D SPIRV_Cross_Combinedt_SkyboxHdrs_linearClampSampler;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 0) out vec4 out_var_SV_TARGET;

void main()
{
    vec3 _28 = normalize(in_var_POSITION);
    vec2 _36 = (vec2(atan(_28.z, _28.x), asin(_28.y)) * vec2(0.159099996089935302734375, 0.3183000087738037109375)) + vec2(0.5);
    _36.y = 1.0 - _36.y;
    out_var_SV_TARGET = vec4(texture(SPIRV_Cross_Combinedt_SkyboxHdrs_linearClampSampler, _36).xyz, 1.0);
}

