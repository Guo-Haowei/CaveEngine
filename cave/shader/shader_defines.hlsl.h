/// File: shader_defines.hlsl.h
#ifndef SHADER_DEFINES_INCLUDED
#define SHADER_DEFINES_INCLUDED

#ifndef MY_PI
#define MY_PI 3.141592653589793f
#endif

#ifndef MY_TWO_PI
#define MY_TWO_PI 6.283185307179586f
#endif

#ifndef CAVE_TRUE
#define CAVE_TRUE  1
#define CAVE_FALSE 0
#endif

#define MAX_LIGHT_COUNT              16
#define MAX_BONE_COUNT               128
#define MAX_POINT_LIGHT_SHADOW_COUNT 8
#define LIGHT_SHADOW_MIN_DISTANCE    0.1f

// light type
#define LIGHT_TYPE_INFINITE 0
#define LIGHT_TYPE_POINT    1
#define LIGHT_TYPE_SPOT     2
#define LIGHT_TYPE_AREA     3
#define LIGHT_TYPE_MAX      4

// display method
#define DISPLAY_CHANNEL_RGB 0
#define DISPLAY_CHANNEL_RRR 1
#define DISPLAY_CHANNEL_AAA 2

// particles
#define PARTICLE_LOCAL_SIZE   32
#define MAX_PARTICLE_COUNT    1048576
#define MAX_FORCE_FIELD_COUNT 64

// mesh defines
#define MESH_HAS_BONE     (1)
#define MESH_HAS_INSTANCE (2)

// SSAO
#define SSAO_KERNEL_SIZE (64)
#define SSAO_NOISE_SIZE  (4)

// compute local sizes
#define COMPUTE_LOCAL_SIZE_VOXEL 4

#if defined(__cplusplus)
#define VCT_CONST constexpr
#elif defined(HLSL_LANG) || defined(__SLANG__) || defined(__TARGET_HLSL__)
#define VCT_CONST static const
#else
#define VCT_CONST const
#endif

VCT_CONST float LUT_SIZE = 64.0;  // ltc_texture size
VCT_CONST float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;
VCT_CONST float LUT_BIAS = 0.5 / LUT_SIZE;

#if defined(__cplusplus)

using uint = unsigned int;
using uint2 = ::cave::math::Vec2u;
using uint3 = ::cave::math::Vec3u;
using uint4 = ::cave::math::Vec4u;

using int2 = ::cave::math::Vec2i;
using int3 = ::cave::math::Vec3i;
using int4 = ::cave::math::Vec4i;

using float2 = ::cave::math::Vec2f;
using float3 = ::cave::math::Vec3f;
using float4 = ::cave::math::Vec4f;

using float4x4 = ::cave::math::Mat4f;

#elif defined(__SLANG__)
// DO NOTHING
#else
#error Unknown shading language
#endif

#endif
