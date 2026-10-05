#version 460

struct _Array_std140_vector_float_4_4
{
    vec4 data[4];
};

struct Light_std140
{
    mat4 projection_matrix;
    mat4 view_matrix;
    _Array_std140_vector_float_4_4 points;
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

struct _Array_std140_Light16
{
    Light_std140 data[16];
};

struct _Array_std140_vector_float_4_64
{
    vec4 data[64];
};

struct ForceField_std140
{
    vec3 position;
    float strength;
};

struct _Array_std140_ForceField64
{
    ForceField_std140 data[64];
};

layout(binding = 5, std140) uniform SLANG_ParameterGroup_PerFrameConstantBuffer_std140
{
    layout(row_major) _Array_std140_Light16 c_lights;
    _Array_std140_vector_float_4_64 c_ssaoKernel;
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
    _Array_std140_ForceField64 c_forceFields;
} PerFrameConstantBuffer;

layout(location = 0) in vec3 input_position;
layout(location = 1) in vec2 input_uv;
layout(location = 2) in vec4 input_color;
layout(location = 0) out vec2 entryPointParam_vs_main_uv;
layout(location = 1) out vec4 entryPointParam_vs_main_color;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    gl_Position = (vec4(input_position, 1.0) * spvWorkaroundRowMajor(PerFrameConstantBuffer.c_camView)) * spvWorkaroundRowMajor(PerFrameConstantBuffer.c_camProj);
    entryPointParam_vs_main_uv = input_uv;
    entryPointParam_vs_main_color = input_color;
}

