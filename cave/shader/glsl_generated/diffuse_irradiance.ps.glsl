#version 460

layout(binding = 7) uniform samplerCube u_Texture0;

layout(location = 0) in vec3 input_world_position;
layout(location = 0) out vec4 entryPointParam_ps_main;

void main()
{
    vec3 _36 = normalize(input_world_position);
    vec3 _38 = cross(vec3(0.0, 1.0, 0.0), _36);
    vec3 _42 = cross(_36, _38);
    float phi = 0.0;
    vec3 irradiance = vec3(0.0);
    float samples = 0.0;
    for (;;)
    {
        if (!(phi < 6.283185482025146484375))
        {
            break;
        }
        float theta = 0.0;
        for (;;)
        {
            if (!(theta < 1.57079637050628662109375))
            {
                break;
            }
            float _57 = theta;
            float _58 = sin(_57);
            float _66 = cos(_57);
            theta += 0.02500000037252902984619140625;
            irradiance += ((textureLod(u_Texture0, ((_38 * (_58 * cos(phi))) + (_42 * (_58 * sin(phi)))) + (_36 * _66), 0.0).xyz * _66) * _58);
            samples += 1.0;
            continue;
        }
        phi += 0.02500000037252902984619140625;
        continue;
    }
    entryPointParam_ps_main = vec4((irradiance * 3.1415927410125732421875) * (1.0 / samples), 1.0);
}

