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
    int _padding1;
    float3 normal;
    int _padding2;
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
    float3 base_color;
    float roughness;
    float3 emissive;
    float metallic;
};

#if defined(__cplusplus)
static_assert(sizeof(GpuPtBvh) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtVertex) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtIndex) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtMesh) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtMaterial) % sizeof(float4) == 0);
#endif