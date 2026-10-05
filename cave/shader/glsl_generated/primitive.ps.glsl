#version 410

uniform sampler2D u_Texture0;

layout(location = 0) in vec2 varying_0;
layout(location = 1) in vec4 varying_1;
layout(location = 0) out vec4 entryPointParam_ps_main;

void main()
{
    vec4 _sampled = texture(u_Texture0, varying_0);
    vec4 color = _sampled * varying_1;
    if (color.w < 0.00999999977648258209228515625)
    {
        discard;
    }
    entryPointParam_ps_main = color;
}

