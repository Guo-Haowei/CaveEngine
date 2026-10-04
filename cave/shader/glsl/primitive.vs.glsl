/// File: primitive.vs.glsl
#include "../cbuffer.hlsl.h"

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_color;

layout(location = 0) out vec2 pass_uv;
layout(location = 1) out vec4 pass_color;

void main() {
    vec4 position = vec4(in_position, 1.0);
    position = c_camView * position;
    position = c_camProj * position;

    gl_Position = position;
    pass_uv = in_uv;
    pass_color = in_color;
}
