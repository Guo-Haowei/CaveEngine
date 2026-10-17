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
