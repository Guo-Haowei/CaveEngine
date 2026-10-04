/// File: skybox.vs.glsl
#version 410
#ifdef GL_ARB_shading_language_420pack
#extension GL_ARB_shading_language_420pack : require
#endif

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

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 0) out vec3 out_var_POSITION;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    gl_Position = (vec4(in_var_POSITION * mat3(spvWorkaroundRowMajor(PerFrameConstantBuffer.c_camView)[0].xyz, spvWorkaroundRowMajor(PerFrameConstantBuffer.c_camView)[1].xyz, spvWorkaroundRowMajor(PerFrameConstantBuffer.c_camView)[2].xyz), 1.0) * spvWorkaroundRowMajor(PerFrameConstantBuffer.c_camProj)).xyww;
    out_var_POSITION = in_var_POSITION;
}

