/// File: ui_overlay.vs.glsl
#include "../cbuffer.hlsl.h"

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_color;

layout(location = 0) out vec2 pass_uv;
layout(location = 1) out vec4 pass_color;

void main() {
    vec2 pos2 = in_position.xy;
    pos2 = pos2 / c_screen_size * 2.0 - 1.0;
    pos2.y = -pos2.y;

    gl_Position = vec4(pos2, 0.0, 1.0);

    pass_uv = in_uv;
    pass_uv.y = 1.0 - pass_uv.y;
    pass_color = in_color;
}