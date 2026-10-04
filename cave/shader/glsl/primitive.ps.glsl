/// File: primitive.ps.glsl

layout(location = 0) in vec2 pass_uv;
layout(location = 1) in vec4 pass_color;

layout(location = 0) out vec4 out_color;

uniform sampler2D u_Texture0;

void main() {
    vec4 sampled_color = texture(u_Texture0, pass_uv, 0);
    //sampled_color *= pass_color;

    if (sampled_color.a < 0.01) {
        discard;
    }

    out_color = sampled_color;
}