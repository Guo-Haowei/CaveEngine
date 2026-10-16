#pragma once
#include "cbuffer.slang.h"

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

#if defined(__cplusplus)
static_assert(sizeof(GpuPtBvh) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtVertex) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtIndex) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtMesh) % sizeof(float4) == 0);
static_assert(sizeof(GpuPtMaterial) % sizeof(float4) == 0);
#endif

#if defined(__cplusplus)
#define SBUFFER(DATA_TYPE, NAME, REG, REG2) \
    static constexpr inline int Get##NAME##Slot() { return REG; }
#else
#define SBUFFER(DATA_TYPE, NAME, REG, REG2) \
    RWStructuredBuffer<DATA_TYPE> NAME : register(u##REG);
#endif

// clang-format off
#define SBUFFER_LIST                                           \
    SBUFFER(ParticleCounter, GlobalParticleCounter,     16, 0) \
    SBUFFER(int,             GlobalDeadIndices,         17, 1) \
    SBUFFER(int,             GlobalAliveIndicesPreSim,  18, 2) \
    SBUFFER(int,             GlobalAliveIndicesPostSim, 19, 3) \
    SBUFFER(Particle,        GlobalParticleData,        20, 4)
// clang-format on

SBUFFER_LIST

#undef SBUFFER
