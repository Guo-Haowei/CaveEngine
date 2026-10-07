/// File: ssao.ps.hlsl
#include "../cbuffer.hlsl.h"
#include "input_output.slang"

#include "common.slang"

TEXTURE_2D(u_Texture0, 0); // t_GbufferNormalMap
TEXTURE_2D(u_Texture1, 1); // t_GbufferDepth
TEXTURE_2D(u_Texture2, 2); // t_NoiseTexture

// @TODO: fix HARD CODE
#define SSAO_KERNEL_BIAS 0.025f

float3 NdcToViewPos2(float2 uv, float depth, float4x4 inv_cam_proj) {
    // Top-left quad UV (0,0) -> NDC (-1.0, +1.0)
    float2 ndc = float2(2.0f * uv.x - 1.0f, 1.0f - 2.0f * uv.y);

#if defined(__TARGET_GLSL__)
    // Remap OpenGL viewport depth [0.5, 1.0] -> NDC clip Z [-1, 1]
    float4 screen_pos = float4(ndc.x, -ndc.y, 2.0f * depth - 1.0f, 1.0f);
#else
    // D3D11 zero-to-one clip depth [0, 1]
    float4 screen_pos = float4(ndc.x, ndc.y, depth, 1.0f);
#endif

    float4 viewPosH = mul(inv_cam_proj, screen_pos);
    return viewPosH.xyz / viewPosH.w;
}

[shader("fragment")]
float ps_main(VS_OUTPUT_UV input) : SV_TARGET {
    const float2 uv = input.uv;

    // 1. Calculate noise scaling for TBN rotation
    int2 texture_size;
    u_Texture0.GetDimensions(texture_size.x, texture_size.y);
    float2 noise_scale = float2(texture_size) * (1.0f / SSAO_NOISE_SIZE);

    // 2. Fetch G-Buffer Normal & decode to [-1, 1]
    float3 N = SAMPLE_TEXTURE_2D(u_Texture0, s_pointClampSampler, uv).rgb;
    N = 2.0f * N - 1.0f;

    // 3. Reconstruct view-space origin from depth
    const float raw_depth = SAMPLE_TEXTURE_2D(u_Texture1, s_pointClampSampler, uv).r;
    const float3 origin = NdcToViewPos2(uv, raw_depth, c_invCamProj);

    // 4. Construct TBN matrix
    float3 rvec = float3(SAMPLE_TEXTURE_LEVEL(u_Texture2, s_pointWrapSampler, uv * noise_scale, 0.0f).xy, 0.0f);
    float3 tangent = normalize(rvec - N * dot(rvec, N));
    float3 bitangent = cross(N, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, N);

    float occlusion = 0.0;

    // 5. Sample kernel loop
    for (int i = 0; i < SSAO_KERNEL_SIZE; ++i) {
        // Transform kernel sample from tangent to view space
        float3 samplePos = mul(c_ssaoKernel[i].xyz, TBN);
        samplePos = origin.xyz + samplePos * c_ssaoKernalRadius;

        // Project view-space sample to clip space
        float4 offset = float4(samplePos, 1.0);
        offset = mul(c_camProj, offset);
        offset.xy /= offset.w;

        // Convert NDC [-1, 1] to Screen UV [0, 1]
#if defined(__TARGET_GLSL__)
        offset.xy = offset.xy * 0.5f + 0.5f;
#else
        offset.xy = float2(offset.x * 0.5f + 0.5f, 0.5f - offset.y * 0.5f);
#endif

        // Fetch depth at re-projected UV & reconstruct view position
        const float depth2 = SAMPLE_TEXTURE_2D(u_Texture1, s_pointClampSampler, offset.xy).r;
        const float3 sampleOcclusionPos = NdcToViewPos2(offset.xy, depth2, c_invCamProj);
        const float sample_depth = sampleOcclusionPos.z;

        // Distance falloff (smoothstep)
        const float range_check = smoothstep(0.0, 1.0, c_ssaoKernalRadius / abs(abs(origin.z) - abs(sample_depth)));

        // Sign-agnostic depth test (handles both +Z and -Z view space)
        const float increment = (abs(sample_depth) <= abs(samplePos.z) - SSAO_KERNEL_BIAS) ? 1.0f : 0.0f;

        occlusion += increment * range_check;
    }

    // 6. Normalize and invert occlusion
    occlusion = 1.0 - (occlusion / float(SSAO_KERNEL_SIZE));
    return occlusion;
}