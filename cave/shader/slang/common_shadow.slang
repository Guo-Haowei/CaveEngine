/// File: shadow.hlsl
#include "../cbuffer.hlsl.h"

// @TODO: refactor this part
// #if defined(HLSL_LANG)

// #define Vector2f       float2
// #define Vector3f       float3
// #define Vector4f       float4
// #define Multiply(a, b) mul((a), (b))
// uint2 textureSizeTexture2D(Texture2D p_texture) {
//     return uint2(width, height);
// }

// #elif defined(GLSL_LANG)

// #define Vector2f       vec2
// #define Vector3f       vec3
// #define Vector4f       vec4
// #define Multiply(a, b) ((a) * (b))
// uvec2 textureSizeTexture2D(sampler2D textureObject) {
//     return textureSize(textureObject, 0);
// }

// vec4 sampleShadow(Texture2D textureObject, vec2 offset) {
//     return texture(textureObject, offset);
// }

// #else
// #error "Unknown shading language"
// #endif

float shadowTest(
#if defined(__TARGET_GLSL__)
                 Sampler2D shadow_map,
#else
                 Texture2D shadow_map,
#endif
                 Light light,
                 float3 worldPos,
                 float NdotL) {
    float4 lightSpacePos = mul(light.view_matrix, float4(worldPos, 1.0));
    lightSpacePos = mul(light.projection_matrix, lightSpacePos);
    lightSpacePos /= lightSpacePos.w;

#if defined(__TARGET_HLSL__)
    lightSpacePos.y = -lightSpacePos.y;
    lightSpacePos.xy = 0.5 * lightSpacePos.xy + 0.5;
#else
    lightSpacePos.xyz = 0.5 * lightSpacePos.xyz + 0.5;
#endif

    float currentDepth = lightSpacePos.z;

    float shadow = 0.0;
    uint width, height;
    shadow_map.GetDimensions(width, height);
    float2 texelSize = 1.0 / float2(width, height);

    // @TODO: better bias
    float bias = max(0.005 * (1.0 - NdotL), 0.0005);

    float2 offsets[4] = {
        float2(0.0, 0.0) * texelSize,
        float2(0.0, 1.0) * texelSize,
        float2(1.0, 0.0) * texelSize,
        float2(1.0, 1.0) * texelSize,
    };

    float d0 = SAMPLE_TEXTURE_LEVEL(shadow_map, s_shadowSampler, lightSpacePos.xy + offsets[0], 0).r;
    float d1 = SAMPLE_TEXTURE_LEVEL(shadow_map, s_shadowSampler, lightSpacePos.xy + offsets[1], 0).r;
    float d2 = SAMPLE_TEXTURE_LEVEL(shadow_map, s_shadowSampler, lightSpacePos.xy + offsets[2], 0).r;
    float d3 = SAMPLE_TEXTURE_LEVEL(shadow_map, s_shadowSampler, lightSpacePos.xy + offsets[3], 0).r;

    shadow +=
        (currentDepth - bias > d0 ? 1.0 : 0.0) +
        (currentDepth - bias > d1 ? 1.0 : 0.0) +
        (currentDepth - bias > d2 ? 1.0 : 0.0) +
        (currentDepth - bias > d3 ? 1.0 : 0.0);

    const float samples = float(4);
    shadow /= samples;
    return shadow;
}
