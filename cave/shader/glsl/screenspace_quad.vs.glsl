/// File: screenspace_quad.vs.glsl
const vec2 positions[6] = vec2[]
(
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0),
    vec2(-1.0, 1.0),
    vec2(-1.0),
    vec2(1.0, -1.0)
);

layout(location = 0) out vec2 pass_uv;

void main()
{
    gl_Position = vec4(positions[uint(gl_VertexID)], 0.0, 1.0);
    pass_uv = (positions[uint(gl_VertexID)] + vec2(1.0)) * 0.5;
}

