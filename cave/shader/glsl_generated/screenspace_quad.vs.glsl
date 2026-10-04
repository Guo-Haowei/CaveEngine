/// File: screenspace_quad.vs.glsl
#version 410
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif

const vec2 _26[6] = vec2[](vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(1.0), vec2(-1.0, 1.0), vec2(-1.0), vec2(1.0, -1.0));

layout(location = 0) out vec2 out_var_TEXCOORD;

void main()
{
    gl_Position = vec4(_26[uint(gl_VertexID)], 0.0, 1.0);
    out_var_TEXCOORD = (_26[uint(gl_VertexID)] + vec2(1.0)) * 0.5;
}

