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

#define CAVE_CBUFFER(NAME, REG, DEF) \
    struct NAME : public ConstantBufferBase<NAME, REG> DEF

// @TODO: remove this constraint
#elif defined(__SLANG__)
#define CAVE_CBUFFER(NAME, REG, DEF) cbuffer NAME : register(b##REG) DEF

#define TextureHandle int4
#endif

#include "cbuffer_list.hlsl.h"

#endif
