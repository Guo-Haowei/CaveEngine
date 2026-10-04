/// File: ui_overlay.vs.glsl
#include "../cbuffer.hlsl.h"
#include "../vsinput.glsl.h"

out vec2 uv;
out vec4 color;

void main()
{
    vec2 pos2 = in_position.xy;
    pos2 = pos2 / c_screen_size * 2.0 - 1.0;
    pos2.y = -pos2.y;

    gl_Position = vec4(pos2, 0.0, 1.0);

    uv = in_uv;
    uv.y = 1.0 - uv.y;
    color = in_color;
}