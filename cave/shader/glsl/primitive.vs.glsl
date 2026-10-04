/// File: primitive.vs.glsl
#include "../cbuffer.hlsl.h"
#include "../vsinput.glsl.h"

layout(location = 0) out vec3 pass_position;

out vec2 uv;
out vec4 color;

void main() {
    vec4 position = vec4(in_position, 1.0);
    position = c_camView * position;
    position = c_camProj * position;

    gl_Position = position;
    uv = in_uv;
    color = in_color;
}
