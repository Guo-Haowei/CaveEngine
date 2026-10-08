/// File: common_shadow.slang
#include "../cbuffer.hlsl.h"

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

#if defined(__TARGET_HLSL__) || defined(__TARGET_METAL__)
    lightSpacePos.y = -lightSpacePos.y;
    lightSpacePos.xy = 0.5 * lightSpacePos.xy + 0.5;
#elif defined(__TARGET_GLSL__)
    lightSpacePos.xyz = 0.5 * lightSpacePos.xyz + 0.5;
#else
#error
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
