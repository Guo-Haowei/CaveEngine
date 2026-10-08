/// File: cbuffer.hlsl.h
#ifndef CBUFFER_INCLUDED
#define CBUFFER_INCLUDED
#include "shader_defines.hlsl.h"

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
#ifdef __TARGET_METAL__
    packed_float3 position;
#else
    float3 position;
#endif
    float strength;
};

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

#define CAVE_CBUFFER(NAME, REG, DEF) \
    struct NAME : public ConstantBufferBase<NAME, REG> DEF

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
#elif defined(HLSL_LANG) || defined(__SLANG__)
#define CAVE_CBUFFER(NAME, REG, DEF) cbuffer NAME : register(b##REG) DEF

#define TextureHandle int2
#define sampler2D     int2
#define samplerCube   int2
#endif

#include "cbuffer_list.hlsl.h"

#endif