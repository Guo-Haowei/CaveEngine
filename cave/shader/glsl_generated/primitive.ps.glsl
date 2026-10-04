/// File: primitive.ps.glsl
#version 450

uniform sampler2D SPIRV_Cross_Combinedt_Sprites_pointWrapSampler;

layout(location = 0) in vec2 in_var_TEXCOORD;
layout(location = 1) in vec4 in_var_COLOR;
layout(location = 0) out vec4 out_var_SV_TARGET;

void main()
{
    vec4 _28 = texture(SPIRV_Cross_Combinedt_Sprites_pointWrapSampler, in_var_TEXCOORD);
    vec4 _29 = _28 * in_var_COLOR;
    if (_29.w < 0.00999999977648258209228515625)
    {
        discard;
    }
    out_var_SV_TARGET = _29;
}

