#version 460

layout(binding = 7) uniform sampler2D u_Texture0;

layout(location = 0) in vec3 input_world_position;
layout(location = 0) out vec4 entryPointParam_ps_main;

void main()
{
    vec3 _10 = normalize(input_world_position);
    vec2 _69 = (vec2(atan(_10.z, _10.x), asin(_10.y)) * vec2(0.159099996089935302734375, 0.3183000087738037109375)) + vec2(0.5);
    _69.y = 1.0 - _69.y;
    entryPointParam_ps_main = vec4(texture(u_Texture0, _69).xyz, 1.0);
}

