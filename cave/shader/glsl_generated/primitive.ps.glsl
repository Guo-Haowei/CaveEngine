#version 460
layout(row_major) uniform;
layout(row_major) buffer;

#line 23 0
layout(binding = 7)
uniform sampler2D u_Texture0;


#line 1095 1
layout(location = 0)
out vec4 entryPointParam_ps_main_0;


#line 1095
layout(location = 0)
in vec2 input_uv_0;


#line 1095
layout(location = 1)
in vec4 input_color_0;


#line 50 0
void main()
{

#line 59
    vec4 color_0 = (texture((u_Texture0), (input_uv_0))) * input_color_0;

    if((color_0.w) < 0.00999999977648258)
    {

#line 62
        discard;

#line 61
    }

#line 61
    entryPointParam_ps_main_0 = color_0;

#line 61
    return;
}

