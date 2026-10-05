#version 460
layout(row_major) uniform;
layout(row_major) buffer;

#line 6 0
struct Light_0
{
    mat4x4 projection_matrix_0;
    mat4x4 view_matrix_0;
    vec4  points_0[4];
    vec3 color_0;
    int type_0;
    vec3 position_0;
    int cast_shadow_0;
    float atten_constant_0;
    float atten_linear_0;
    float atten_quadratic_0;
    float max_distance_0;
    vec3 padding_0;
    int shadow_map_index_0;
};


#line 27
struct ForceField_0
{
    vec3 position_1;
    float strength_0;
};


#line 4 1
struct SLANG_ParameterGroup_PerPassConPerFrameConstantBuffer_0
{
    Light_0  c_lights[16];
    vec4  c_ssaoKernel[64];
    mat4x4 c_camProj;
    mat4x4 c_camView;
    mat4x4 c_invCamProj;
    mat4x4 c_invCamView;
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
    ForceField_0  c_forceFields[64];
};


#line 4
layout(binding = 5)
layout(std140) uniform block_SLANG_ParameterGroup_PerPassConPerFrameConstantBuffer_0
{
    Light_0  c_lights[16];
    vec4  c_ssaoKernel[64];
    mat4x4 c_camProj;
    mat4x4 c_camView;
    mat4x4 c_invCamProj;
    mat4x4 c_invCamView;
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
    ForceField_0  c_forceFields[64];
}PerPassConPerFrameConstantBuffer;

#line 79
layout(location = 0)
out vec2 entryPointParam_vs_main_uv_0;


#line 79
layout(location = 1)
out vec4 entryPointParam_vs_main_color_0;


#line 79
layout(location = 0)
in vec3 input_position_0;


#line 79
layout(location = 1)
in vec2 input_uv_0;


#line 79
layout(location = 2)
in vec4 input_color_0;


#line 75
struct VSOutput_0
{
    vec4 position_2;
    vec2 uv_0;
    vec4 color_1;
};


#line 94
void main()
{
    VSOutput_0 output_0;

#line 102
    output_0.position_2 = ((((((vec4(input_position_0, 1.0)) * (PerPassConPerFrameConstantBuffer.c_camView)))) * (PerPassConPerFrameConstantBuffer.c_camProj)));
    output_0.uv_0 = input_uv_0;
    output_0.color_1 = input_color_0;

    VSOutput_0 _S1 = output_0;

#line 106
    gl_Position = output_0.position_2;

#line 106
    entryPointParam_vs_main_uv_0 = _S1.uv_0;

#line 106
    entryPointParam_vs_main_color_0 = _S1.color_1;

#line 106
    return;
}

