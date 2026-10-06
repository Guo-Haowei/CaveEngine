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

float shadowTest(Texture2D shadowMap,
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
    shadowMap.GetDimensions(width, height);
    float2 texelSize = 1.0 / float2(width, height);

    // @TODO: better bias
    float bias = max(0.005 * (1.0 - NdotL), 0.0005);

    // @TODO: refactor
    #define SHADOW_SAMPLES 4
    [unroll(SHADOW_SAMPLES)]
    for (uint sample = 0; sample < SHADOW_SAMPLES; ++sample) {
        uint x = sample / 2;
        uint y = sample % 2;
        float2 offset = float2(x, y) * texelSize;

        float closestDepth = SAMPLE_TEXTURE_2D(shadowMap, s_shadowSampler, lightSpacePos.xy + offset).r;

        shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
    }

    const float samples = float(4);
    shadow /= samples;
    return shadow;
}
