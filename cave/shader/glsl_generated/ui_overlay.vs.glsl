/// File: ui_overlay.vs.glsl
#version 450

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
layout(location = 1) in vec2 in_var_TEXCOORD;
layout(location = 2) in vec4 in_var_COLOR;
layout(location = 0) out vec2 out_var_TEXCOORD;
layout(location = 1) out vec4 out_var_COLOR;

void main()
{
    vec2 _42 = in_var_TEXCOORD;
    vec2 _49 = ((in_var_POSITION.xy / PerFrameConstantBuffer.c_screen_size) * 2.0) - vec2(1.0);
    _42.y = 1.0 - _42.y;
    gl_Position = vec4(_49.x, -_49.y, 0.0, 1.0);
    out_var_TEXCOORD = _42;
    out_var_COLOR = in_var_COLOR;
}

