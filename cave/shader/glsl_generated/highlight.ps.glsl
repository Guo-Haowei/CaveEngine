/// File: highlight.ps.glsl
#version 410
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif

layout(location = 0) out float out_var_SV_TARGET;

void main()
{
    out_var_SV_TARGET = 1.0;
}

