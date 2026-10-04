/// File: primitive.ps.glsl

in vec2 uv;
in vec4 color;

uniform sampler2D t_Sprite;

layout(location = 0) out vec4 out_color;

void main()
{
    vec4 sampled_color = texture(t_Sprite, uv);
    sampled_color *= color;

    if (sampled_color.a < 0.01) {
        discard;
    }

    out_color = sampled_color;
}