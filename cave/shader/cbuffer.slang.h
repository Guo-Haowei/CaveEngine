/// File: cbuffer.slang.h
#ifndef CBUFFER_INCLUDED
#define CBUFFER_INCLUDED
#include "shader_defines.hlsl.h"

struct Light {
    float4x4 projection_matrix;  // 64
    float4x4 view_matrix;        // 64
    float4 points[4];            // 64

    float4 color;

    float4 position;  // direction

    float atten_constant;
    float atten_linear;

    float atten_quadratic;
    float max_distance;  // max distance the light affects

    int cast_shadow;
    int type;
    int shadow_map_index;
    int padding;
};

struct ForceField {
    float4 position_strengh;
};

// constant buffer
#if defined(__cplusplus)
struct alignas(16) TextureHandle {
    uint64_t value;
    uint64_t _padding;
};

static_assert(alignof(TextureHandle) == 16);
static_assert(sizeof(TextureHandle) == 16);

template<typename T, int N>
struct ConstantBufferBase {
    ConstantBufferBase() {
        static_assert(sizeof(T) % 16 == 0);
    }
    constexpr int GetSlot() { return N; }
    static constexpr int GetUniformBufferSlot() { return N; }
};

#define CAVE_CBUFFER(NAME, REG, DEF)                        \
    struct NAME : public ConstantBufferBase<NAME, REG> DEF; \
    static_assert(sizeof(NAME) % 256 == 0)

// @TODO: remove this constraint
#elif defined(__SLANG__)
#define CAVE_CBUFFER(NAME, REG, DEF) cbuffer NAME : register(b##REG) DEF

#define TextureHandle int4
#endif

CAVE_CBUFFER(PerBatchConstantBuffer, 0, {
    float4x4 c_world_matrix;
    float4x4 c_cube_proj_view_matrix;

    float4x4 _per_batch_padding_0;

    // reuse per batch buffer for bloom
    float4 c_tint_color;
    float4 c_uv_rect;
    float4 _per_batch_padding_1;

    float c_env_pass_roughness;  // for environment map
    int c_mesh_flag;
    int _per_batch_padding_2;
    int _per_batch_padding_3;
});

CAVE_CBUFFER(PerViewConstantBuffer, 1, {
    float4x4 c_viewMatrix;
    float4x4 c_projectionMatrix;

    float4x4 _per_view_padding_0;
    float4x4 _per_view_padding_1;
});

CAVE_CBUFFER(MaterialConstantBuffer, 2, {
    // 16
    float4 c_baseColor;

    float c_metallic;
    float c_roughness;
    float c_reflect_power;
    float c_emissive_power;

    int c_has_base_color_map;
    int c_has_material_map;
    int c_has_normal_map;
    int c_has_height_map;

    uint c_base_color_map_index;
    uint c_normal_map_index;
    uint c_material_map_index;
    uint c_height_map_index;

    // 16
    TextureHandle c_base_color_map_handle;
    TextureHandle c_normal_map_handle;
    TextureHandle c_material_map_handle;
    TextureHandle c_height_map_handle;

    // 16
    float4x4 _material_padding_3;

    // 16
    float4x4 _material_padding_4;
});

CAVE_CBUFFER(PerPassConstantBuffer, 3, {
    uint4 c_pass_textures[15];

    uint c_output_uav0;
    uint c_output_uav1;
    uint c_output_uav2;
    uint c_output_uav3;
});

// @TODO: change to unordered access buffer
CAVE_CBUFFER(BoneConstantBuffer, 4, {
    float4x4 c_bones[MAX_BONE_COUNT];
});

CAVE_CBUFFER(PerFrameConstantBuffer, 5, {
    Light c_lights[MAX_LIGHT_COUNT];

    float4 c_ssaoKernel[SSAO_KERNEL_SIZE];
    //-----------------------------------------

    float4x4 c_cam_proj;
    float4x4 c_cam_view;
    float4x4 c_inv_cam_proj;
    float4x4 c_inv_cam_view;

    float4 c_cam_pos;
    float4 c_cam_forward;
    float4 c_cam_right;
    float4 c_cam_up;

    //-----------------------------------------
    int c_lightCount;
    int c_enableBloom;
    int c_debugCsm;
    int c_iblEnabled;

    int c_debugVoxelId;
    int c_ssaoEnabled;
    int c_enableVxgi;
    int c_ptObjectCount;

    float c_cam_fovy;  // 16
    float c_texelSize;
    float c_bloomThreshold;
    float c_ssaoKernalRadius;

    uint c_DiffuseIrradianceResidentHandle;
    uint c_PrefilteredResidentHandle;
    uint c_BrdfLutResidentHandle;
    int c_forceFieldsCount;  // 16
    //-----------------------------------------

    float4 c_screen_size;
    float4 c_ambientColor;  // 16
    float4 _per_frame_padding_1;

    uint c_frame_index;
    float c_voxelSize;
    int c_scene_dirty;
    int _per_frame_padding_2;

    float4x4 _per_frame_padding_3;

    // float3 c_voxelWorldCenter;
    // float c_voxelWorldSizeHalf;  // 16

    ForceField c_forceFields[MAX_FORCE_FIELD_COUNT];
});

CAVE_CBUFFER(EmitterConstantBuffer, 6, {
    float4 c_particleColor;
    float3 c_seeds;
    float c_emitterScale;
    float3 c_emitterPosition;
    int c_particlesPerFrame;
    float3 c_emitterStartingVelocity;
    int c_emitterMaxParticleCount;

    int c_preSimIdx;
    int c_postSimIdx;
    float c_elapsedTime;
    float c_lifeSpan;

    int2 c_emitterSubUv;
    int c_emitterUseTexture;
    int c_emitterHasGravity;

    float3 _emitter_padding_2;
    int c_subUvCounter;

    float4 _emitter_padding_3;
    float4x4 _emitter_padding_4;
    float4x4 _emitter_padding_5;
});

CAVE_CBUFFER(PointShadowConstantBuffer, 7, {
    float4x4 c_pointLightMatrix;  // 64
    float3 c_pointLightPosition;  // 12
    float c_pointLightFar;        // 4

    float4 _point_shadow_padding_0;  // 16
    float4 _point_shadow_padding_1;  // 16
    float4 _point_shadow_padding_2;  // 16

    float4x4 _point_shadow_padding_3;  // 64
    float4x4 _point_shadow_padding_4;  // 64
});

#endif
