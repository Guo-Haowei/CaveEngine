#version 410

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

layout(std140) uniform SLANG_ParameterGroup_PerFrameConstantBuffer_std140
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
layout(location = 0) out vec2 varying_0;
layout(location = 1) out vec4 varying_1;

void main()
{
    vec2 _46 = ((input_position.xy / PerFrameConstantBuffer.c_screen_size) * 2.0) - vec2(1.0);
    _46.y = -_46.y;
    vec2 _62 = input_uv;
    _62.y = 1.0 - _62.y;
    gl_Position = vec4(_46, 0.0, 1.0);
    varying_0 = _62;
    varying_1 = input_color;
}

