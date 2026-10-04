/// File: bloom_upsample.cs.glsl
#version 410
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif
#extension GL_ARB_compute_shader : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_shader_image_size : require
#extension GL_EXT_shader_image_load_formatted : require
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0) uniform image2D u_BloomOutputImage;
uniform sampler2D SPIRV_Cross_Combinedt_BloomInputTextureSPIRV_Cross_DummySampler;
uniform sampler2D SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler;

void main()
{
    uvec2 _39 = uvec2(imageSize(u_BloomOutputImage));
    uvec2 _51 = uvec2(textureSize(SPIRV_Cross_Combinedt_BloomInputTextureSPIRV_Cross_DummySampler, 0));
    float _57 = (float(gl_GlobalInvocationID.x) / float(_39.x)) + (0.5 / float(_51.x));
    float _59 = (float(gl_GlobalInvocationID.y) / float(_39.y)) + (0.5 / float(_51.y));
    float _62 = _57 - 0.004999999888241291046142578125;
    float _63 = _59 + 0.004999999888241291046142578125;
    float _76 = _57 + 0.004999999888241291046142578125;
    float _101 = _59 - 0.004999999888241291046142578125;
    imageStore(u_BloomOutputImage, ivec2(gl_GlobalInvocationID.xy), mix(imageLoad(u_BloomOutputImage, ivec2(gl_GlobalInvocationID.xy)).xyz, (((textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_57, _59), 0.0).xyz * 4.0) + ((((textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_57, _63), 0.0).xyz + textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_62, _59), 0.0).xyz) + textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_76, _59), 0.0).xyz) + textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_57, _101), 0.0).xyz) * 2.0)) + (((textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_62, _63), 0.0).xyz + textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_76, _63), 0.0).xyz) + textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_62, _101), 0.0).xyz) + textureLod(SPIRV_Cross_Combinedt_BloomInputTextures_linearClampSampler, vec2(_76, _101), 0.0).xyz)) * 0.0625, vec3(0.60000002384185791015625)).xyzz);
}

