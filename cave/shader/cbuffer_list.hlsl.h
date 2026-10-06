#ifndef CAVE_CBUFFER
#error "CAVE_CBUFFER NOT DEFINED"
#endif

CAVE_CBUFFER(PerBatchConstantBuffer, 0, {
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
});

CAVE_CBUFFER(PerPassConstantBuffer, 1, {
    float4x4 c_viewMatrix;
    float4x4 c_projectionMatrix;

    float4x4 _per_pass_padding_0;
    float4x4 _per_pass_padding_1;
});

CAVE_CBUFFER(MaterialConstantBuffer, 2, {
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
});

// @TODO: change to unordered access buffer
CAVE_CBUFFER(BoneConstantBuffer, 3, {
    float4x4 c_bones[MAX_BONE_COUNT];
});

CAVE_CBUFFER(PointShadowConstantBuffer, 4, {
    float4x4 c_pointLightMatrix;  // 64
    float3 c_pointLightPosition;  // 12
    float c_pointLightFar;        // 4

    float4 _point_shadow_padding_0;  // 16
    float4 _point_shadow_padding_1;  // 16
    float4 _point_shadow_padding_2;  // 16

    float4x4 _point_shadow_padding_3;  // 64
    float4x4 _point_shadow_padding_4;  // 64
});

CAVE_CBUFFER(PerFrameConstantBuffer, 5, {
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

    float2 c_screen_size;
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
