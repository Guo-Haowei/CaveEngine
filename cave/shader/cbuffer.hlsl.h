/// File: cbuffer.hlsl.h
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

struct sampler_t {
    union {
        int4 handle_d3d;
        uint64_t handle_gl;
    };
    sampler_t() { handle_gl = 0; }

    void Set32(int p_value) { handle_d3d.x = handle_d3d.y = p_value; }
    void Set64(uint64_t p_value) { handle_gl = p_value; }
};

static_assert(sizeof(sampler_t) == sizeof(int4));

using sampler3D = sampler_t;
using samplerCube = sampler_t;

// @TODO: remove this constraint
#elif defined(__SLANG__)
#define CAVE_CBUFFER(NAME, REG, DEF) cbuffer NAME : register(b##REG) DEF

#define TextureHandle int4
#define sampler2D     int4
#define samplerCube   int4
#endif

#include "cbuffer_list.hlsl.h"

#endif
