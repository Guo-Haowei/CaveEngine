/// File: bloom_setup.cs.glsl
#version 450
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

struct Light
{
    mat4 projection_matrix;
    mat4 view_matrix;
    vec4 points[4];
    vec3 color;
    int type;
    vec3 position;
    int cast_shadow;
    float atten_constant;
    float atten_linear;
    float atten_quadratic;
    float max_distance;
    vec3 padding;
    int shadow_map_index;
};

struct ForceField
{
    vec3 position;
    float strength;
};

layout(binding = 5, std140) uniform type_PerFrameConstantBuffer
{
    layout(row_major) Light c_lights[16];
    vec4 c_ssaoKernel[64];
    layout(row_major) mat4 c_camProj;
    layout(row_major) mat4 c_camView;
    layout(row_major) mat4 c_invCamProj;
    layout(row_major) mat4 c_invCamView;
    vec4 _per_frame_padding_2;
    vec4 _per_frame_padding_3;
    vec4 _per_frame_padding_4;
    vec3 c_sunPosition;
    int c_iblEnabled;
    vec4 c_ambientColor;
    int c_lightCount;
    int c_enableBloom;
    int c_debugCsm;
    float c_bloomThreshold;
    int c_debugVoxelId;
    int c_ssaoEnabled;
    int c_enableVxgi;
    float c_texelSize;
    vec2 c_screen_size;
    float c_ssaoKernalRadius;
    int c_ptObjectCount;
    uint c_DiffuseIrradianceResidentHandle;
    uint c_PrefilteredResidentHandle;
    uint c_BrdfLutResidentHandle;
    int c_forceFieldsCount;
    vec4 _c_SkyboxHdrResidentHandle;
    vec4 _c_ShadowMapResidentHandle;
    vec3 c_cameraPosition;
    float c_camera_fovy;
    vec3 c_voxelWorldCenter;
    float c_voxelWorldSizeHalf;
    vec3 c_cameraForward;
    uint c_frame_index;
    vec3 c_cameraRight;
    int c_scene_dirty;
    vec3 c_cameraUp;
    float c_voxelSize;
    ForceField c_forceFields[64];
} PerFrameConstantBuffer;

layout(binding = 0) uniform writeonly image2D u_BloomOutputImage;
uniform sampler2D SPIRV_Cross_Combinedt_TextureLightings_linearClampSampler;

void main()
{
    uvec2 _51 = uvec2(imageSize(u_BloomOutputImage));
    vec3 _67 = textureLod(SPIRV_Cross_Combinedt_TextureLightings_linearClampSampler, vec2(float(gl_GlobalInvocationID.x) / float(_51.x), float(gl_GlobalInvocationID.y) / float(_51.y)), 0.0).xyz;
    vec3 _68 = mix(mix(max(_67, vec3(0.0)), vec3(0.0), isnan(_67)), _67, isnan(vec3(0.0)));
    float _70 = dot(mix(mix(max(_68, vec3(0.0)), vec3(0.0), isnan(_68)), _68, isnan(vec3(0.0))), vec3(0.2125999927520751953125, 0.715200006961822509765625, 0.072200000286102294921875));
    float _73 = _70 - PerFrameConstantBuffer.c_bloomThreshold;
    imageStore(u_BloomOutputImage, ivec2(gl_GlobalInvocationID.xy), (_68 * ((isnan(0.0) ? _73 : (isnan(_73) ? 0.0 : max(_73, 0.0))) / (isnan(9.9999997473787516355514526367188e-06) ? _70 : (isnan(_70) ? 9.9999997473787516355514526367188e-06 : max(_70, 9.9999997473787516355514526367188e-06))))).xyzz);
}

