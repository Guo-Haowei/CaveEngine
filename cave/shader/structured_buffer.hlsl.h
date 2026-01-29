#ifndef STRUCTURED_BUFFER_HLSL_H_INCLUDED
#define STRUCTURED_BUFFER_HLSL_H_INCLUDED
#include "cbuffer.hlsl.h"

struct Particle {
    float4 position;
    float4 velocity;
    float4 color;

    float scale;
    float lifeSpan;
    float lifeRemaining;
    int isActive;
};

struct ParticleCounter {
    int aliveCount[2];
    int deadCount;
    int simulationCount;
    int emissionCount;
};

// ray tracing
struct GpuPtBvh {
    float3 min;
    int missIdx;
    float3 max;
    int hitIdx;

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
    float4x4 transformInv;

    float2 _padding4;
    int rootBvhId;
    int materialId;
};

struct GpuPtMaterial {
    float3 baseColor;
    float roughness;
    float3 emissive;
    float metallic;
};

#ifdef __cplusplus
static_assert(sizeof(GpuPtBvh) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtVertex) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtIndex) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtMesh) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtMaterial) % sizeof(float4) == 0);
#endif  // __cplusplus

#define SBUFFER_LIST                                         \
    SBUFFER(ParticleCounter, GlobalParticleCounter, 16, 511) \
    SBUFFER(int, GlobalDeadIndices, 17, 510)                 \
    SBUFFER(int, GlobalAliveIndicesPreSim, 18, 509)          \
    SBUFFER(int, GlobalAliveIndicesPostSim, 19, 508)         \
    SBUFFER(Particle, GlobalParticleData, 20, 507)           \
    SBUFFER(GpuPtVertex, GlobalPtVertices, 21, 506)          \
    SBUFFER(GpuPtIndex, GlobalPtIndices, 22, 505)            \
    SBUFFER(GpuPtBvh, GlobalPtBvhs, 23, 504)                 \
    SBUFFER(GpuPtMesh, GlobalPtMeshes, 24, 503)              \
    SBUFFER(GpuPtMaterial, GlobalPtMaterials, 25, 502)

#endif
