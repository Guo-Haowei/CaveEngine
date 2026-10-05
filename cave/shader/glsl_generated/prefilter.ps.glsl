#version 460

layout(binding = 0, std140) uniform SLANG_ParameterGroup_PerBatchConstantBuffer_std140
{
    layout(row_major) mat4 c_worldMatrix;
    vec4 _dummy;
    vec2 _per_batch_padding_0;
    float c_envPassRoughness;
    int c_meshFlag;
    vec4 c_tint_color;
    vec4 c_uv_rect;
    layout(row_major) mat4 c_cubeProjectionViewMatrix;
    layout(row_major) mat4 _per_batch_padding_5;
} PerBatchConstantBuffer;

layout(binding = 7) uniform samplerCube u_Texture0;

layout(location = 0) in vec3 input_world_position;
layout(location = 0) out vec4 entryPointParam_ps_main;

void main()
{
    vec3 _33 = normalize(input_world_position);
    uint i = 0u;
    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;
    float mipLevel;
    vec3 _298;
    vec3 _299;
    for (;;)
    {
        if (!(i < 1024u))
        {
            break;
        }
        uint _275 = (i << 16u) | (i >> 16u);
        uint _280 = ((_275 & 1431655765u) << 1u) | ((_275 & 2863311530u) >> 1u);
        uint _285 = ((_280 & 858993459u) << 2u) | ((_280 & 3435973836u) >> 2u);
        uint _290 = ((_285 & 252645135u) << 4u) | ((_285 & 4042322160u) >> 4u);
        vec2 _270 = vec2(float(i) * 0.0009765625, float(((_290 & 16711935u) << 8u) | ((_290 & 4278255360u) >> 8u)) * 2.3283064365386962890625e-10);
        float _302 = PerBatchConstantBuffer.c_envPassRoughness * PerBatchConstantBuffer.c_envPassRoughness;
        float _304 = 6.283185482025146484375 * _270.x;
        float _305 = _270.y;
        float _312 = sqrt((1.0 - _305) / (1.0 + (((_302 * _302) - 1.0) * _305)));
        float _315 = sqrt(1.0 - (_312 * _312));
        vec3 _358 = _298;
        _358.x = cos(_304) * _315;
        _358.y = sin(_304) * _315;
        _358.z = _312;
        _298 = _358;
        if (abs(_33.z) < 0.999000012874603271484375)
        {
            _299 = vec3(0.0, 0.0, 1.0);
        }
        else
        {
            _299 = vec3(1.0, 0.0, 0.0);
        }
        vec3 _331 = normalize(cross(_299, _33));
        vec3 _344 = normalize(((_331 * _298.x) + (cross(_33, _331) * _298.y)) + (_33 * _298.z));
        vec3 _184 = reflect(-_33, _344);
        float _185 = dot(_33, _184);
        if (_185 > 0.0)
        {
            float _189 = max(dot(_33, _344), 0.0);
            float _347 = PerBatchConstantBuffer.c_envPassRoughness * PerBatchConstantBuffer.c_envPassRoughness;
            float _348 = _347 * _347;
            float _352 = ((_189 * _189) * (_348 - 1.0)) + 1.0;
            if (PerBatchConstantBuffer.c_envPassRoughness == 0.0)
            {
                mipLevel = 0.0;
            }
            else
            {
                mipLevel = 0.5 * log2(125164.5390625 / ((1024.0 * ((((_348 / ((3.1415927410125732421875 * _352) * _352)) * _189) / (4.0 * max(dot(_344, _33), 0.0))) + 9.9999997473787516355514526367188e-05)) + 9.9999997473787516355514526367188e-05));
            }
            prefilteredColor += (textureLod(u_Texture0, _184, mipLevel).xyz * _185);
            totalWeight += _185;
        }
        i++;
        continue;
    }
    entryPointParam_ps_main = vec4(prefilteredColor / vec3(totalWeight), 1.0);
}

