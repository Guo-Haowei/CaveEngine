/// File: cbuffer.hlsl.h
#ifndef CBUFFER_INCLUDED
#define CBUFFER_INCLUDED
#include "shader_defines.hlsl.h"

// constant buffer
#if defined(__cplusplus)
using TextureHandle = uint64_t;

template<typename T, int N>
struct ConstantBufferBase {
    ConstantBufferBase() {
        static_assert(sizeof(T) % 16 == 0);
    }
    constexpr int GetSlot() { return N; }
    static constexpr int GetUniformBufferSlot() { return N; }
};

#define CBUFFER(NAME, REG) \
    struct NAME : public ConstantBufferBase<NAME, REG>

struct sampler_t {
    union {
        int2 handle_d3d;
        uint64_t handle_gl;
    };
    sampler_t() { handle_gl = 0; }

    void Set32(int p_value) { handle_d3d.x = handle_d3d.y = p_value; }
    void Set64(uint64_t p_value) { handle_gl = p_value; }
};

static_assert(sizeof(sampler_t) == sizeof(uint64_t));

using sampler3D = sampler_t;
using samplerCube = sampler_t;

// @TODO: remove this constraint
#elif defined(HLSL_LANG)
#define CBUFFER(NAME, REG) cbuffer NAME : register(b##REG)

#define TextureHandle int2
#define sampler2D     int2
#define samplerCube   int2

#elif defined(GLSL_LANG)
#define CBUFFER(NAME, REG) layout(std140, binding = REG) uniform NAME

#define TextureHandle vec2

#endif

struct Light {
    float4x4 projection_matrix;  // 64
    float4x4 view_matrix;        // 64
    float4 points[4];            // 64

    float3 color;
    int type;

    float3 position;  // direction
    int cast_shadow;

    float atten_constant;
    float atten_linear;

    float atten_quadratic;
    float max_distance;  // max distance the light affects

    float3 padding;
    int shadow_map_index;
};

struct ForceField {
    float3 position;
    float strength;
};

CBUFFER(PerBatchConstantBuffer, 0) {
    float4x4 c_worldMatrix;

    // reuse per batch buffer for bloom
    float4 _dummy;

    float2 _per_batch_padding_0;
    float c_envPassRoughness;  // for environment map
    int c_meshFlag;

    float4 c_tint_color;
    float4 c_uv_rect;

    float4x4 c_cubeProjectionViewMatrix;
    float4x4 _per_batch_padding_5;
};

CBUFFER(PerPassConstantBuffer, 1) {
    float4x4 c_viewMatrix;
    float4x4 c_projectionMatrix;

    float4x4 _per_pass_padding_0;
    float4x4 _per_pass_padding_1;
};

CBUFFER(MaterialConstantBuffer, 2) {
    // 16 floats
    float4 c_baseColor;

    float3 _material_padding_0;
    int c_displayChannel;

    float c_metallic;
    float c_roughness;
    float c_reflectPower;
    float c_emissivePower;

    int c_hasBaseColorMap;
    int c_hasMaterialMap;
    int c_hasNormalMap;
    int c_hasHeightMap;

    // 16 floats
    TextureHandle c_baseColorMapHandle;
    TextureHandle c_normalMapHandle;
    TextureHandle c_materialMapHandle;
    TextureHandle c_heightMapHandle;

    float4 _material_padding_1;
    float4 _material_padding_2;

    // 16 floats
    float4x4 _material_padding_3;

    // 16 floats
    float4x4 _material_padding_4;
};

// @TODO: change to unordered access buffer
CBUFFER(BoneConstantBuffer, 3) {
    float4x4 c_bones[MAX_BONE_COUNT];
};

CBUFFER(PointShadowConstantBuffer, 4) {
    float4x4 c_pointLightMatrix;  // 64
    float3 c_pointLightPosition;  // 12
    float c_pointLightFar;        // 4

    float4 _point_shadow_padding_0;  // 16
    float4 _point_shadow_padding_1;  // 16
    float4 _point_shadow_padding_2;  // 16

    float4x4 _point_shadow_padding_3;  // 64
    float4x4 _point_shadow_padding_4;  // 64
};

CBUFFER(PerFrameConstantBuffer, 5) {
    Light c_lights[MAX_LIGHT_COUNT];

    float4 c_ssaoKernel[SSAO_KERNEL_SIZE];
    //-----------------------------------------
    float4x4 c_camProj;
    float4x4 c_camView;
    float4x4 c_invCamProj;
    float4x4 c_invCamView;

    float4 _per_frame_padding_2;
    float4 _per_frame_padding_3;
    float4 _per_frame_padding_4;
    float3 c_sunPosition;
    int c_iblEnabled;
    //-----------------------------------------
    float4 c_ambientColor;  // 16

    int c_lightCount;
    int c_enableBloom;
    int c_debugCsm;
    float c_bloomThreshold;  // 16

    int c_debugVoxelId;
    int c_ssaoEnabled;
    int c_enableVxgi;
    float c_texelSize;  // 16

    int2 c_tileOffset;
    float c_ssaoKernalRadius;
    int c_ptObjectCount;
    //-----------------------------------------
    uint c_DiffuseIrradianceResidentHandle;
    uint c_PrefilteredResidentHandle;
    uint c_BrdfLutResidentHandle;
    int c_forceFieldsCount;  // 16

    float4 _c_SkyboxHdrResidentHandle;  // 16
    float4 _c_ShadowMapResidentHandle;

    float3 c_cameraPosition;
    float c_camera_fovy;  // 16
    //-----------------------------------------
    float3 c_voxelWorldCenter;
    float c_voxelWorldSizeHalf;  // 16

    float3 c_cameraForward;
    uint c_frame_index;  // 16

    float3 c_cameraRight;
    int c_scene_dirty;  // 16

    float3 c_cameraUp;
    float c_voxelSize;  // 16
    //-----------------------------------------

    ForceField c_forceFields[MAX_FORCE_FIELD_COUNT];
};

CBUFFER(EmitterConstantBuffer, 6) {
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
};

#endif