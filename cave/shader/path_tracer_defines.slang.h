/// File: path_tracer_defines.hlsl.h
#pragma once

struct GpuPtBvh {
    float3 min;
    int miss_idx;
    float3 max;
    int hit_idx;

    float2 _padding;
    int leaf;
    int triangleIndex;
};

struct GpuPtVertex {
    float3 position;
    float u;
    float3 normal;
    float v;
};

struct GpuPtIndex {
    int3 tri;
    int _padding1;
};

struct GpuPtMesh {
    float4x4 transform;
    float4x4 transform_inv;

    float2 _padding4;
    int root_bvh_id;
    int material_idx;
};

struct GpuPtMaterial {
    float4 base_color;

    float roughness;
    float emissive;
    float metallic;
    float padding;

    uint base_color_map_handle;
    uint normal_map_handle;
    uint material_map_handle;
    uint height_map_handle;
};

#if defined(__cplusplus)
static_assert(sizeof(GpuPtBvh) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtVertex) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtIndex) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtMesh) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtMaterial) % sizeof(float4) == 0);
#endif