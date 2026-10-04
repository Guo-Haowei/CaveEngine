/// File: bloom_downsample.cs.glsl
#version 450
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0) uniform writeonly image2D u_BloomOutputImage;
uniform sampler2D SPIRV_Cross_Combinedt_BloomInputTextureSPIRV_Cross_DummySampler;
uniform sampler2D SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler;

void main()
{
    uvec2 _38 = uvec2(imageSize(u_BloomOutputImage));
    uvec2 _50 = uvec2(textureSize(SPIRV_Cross_Combinedt_BloomInputTextureSPIRV_Cross_DummySampler, 0));
    float _53 = float(_50.x);
    float _54 = 1.0 / _53;
    float _55 = float(_50.y);
    float _56 = 1.0 / _55;
    float _58 = (float(gl_GlobalInvocationID.x) / float(_38.x)) + (0.5 / _53);
    float _60 = (float(gl_GlobalInvocationID.y) / float(_38.y)) + (0.5 / _55);
    float _63 = 2.0 / _53;
    float _64 = _58 - _63;
    float _65 = 2.0 / _55;
    float _66 = _60 + _65;
    float _79 = _58 + _63;
    float _104 = _60 - _65;
    float _123 = _58 - _54;
    float _124 = _60 + _56;
    float _131 = _58 + _54;
    float _138 = _60 - _56;
    vec3 _164 = (((textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_58, _60), 0.0).xyz * 0.125) + ((((textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_64, _66), 0.0).xyz + textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_79, _66), 0.0).xyz) + textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_64, _104), 0.0).xyz) + textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_79, _104), 0.0).xyz) * 0.03125)) + ((((textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_58, _66), 0.0).xyz + textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_64, _60), 0.0).xyz) + textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_79, _60), 0.0).xyz) + textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_58, _104), 0.0).xyz) * 0.0625)) + ((((textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_123, _124), 0.0).xyz + textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_131, _124), 0.0).xyz) + textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_123, _138), 0.0).xyz) + textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_131, _138), 0.0).xyz) * 0.125);
    imageStore(u_BloomOutputImage, ivec2(gl_GlobalInvocationID.xy), mix(mix(max(_164, vec3(0.0)), vec3(0.0), isnan(_164)), _164, isnan(vec3(0.0))).xyzz);
}

