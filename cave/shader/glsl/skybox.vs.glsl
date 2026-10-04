/// File: skybox.vs.glsl
#include "../cbuffer.hlsl.h"
#include "../vsinput.glsl.h"

layout(location = 0) out vec3 pass_position;

void main() {
    mat3 rotate = mat3(c_camView);
    vec3 position3 = rotate * in_position;
    vec4 position = c_camProj * vec4(position3, 1.0);

    gl_Position = position.xyww;
    pass_position = in_position;
}