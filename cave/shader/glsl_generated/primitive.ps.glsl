#version 460

layout(binding = 7) uniform sampler2D u_Texture0;

layout(location = 0) in vec2 input_uv;
layout(location = 1) in vec4 input_color;
layout(location = 0) out vec4 entryPointParam_ps_main;

void main()
{
    vec4 _sampled = texture(u_Texture0, input_uv);
    vec4 color = _sampled * input_color;
    if (color.w < 0.00999999977648258209228515625)
    {
        discard;
    }
    entryPointParam_ps_main = color;
}

