/// File: ssao.ps.hlsl
#include "../cbuffer.hlsl.h"
#include "input_output.slang"

#include "common.slang"

TEXTURE_2D(u_Texture0, 0); // t_GbufferNormalMap
TEXTURE_2D(u_Texture1, 1); // t_GbufferDepth
TEXTURE_2D(u_Texture2, 2); // t_NoiseTexture

// @TODO: fix HARD CODE
#define SSAO_KERNEL_BIAS 0.025f

[shader("fragment")]
float ps_main(VS_OUTPUT_UV input) : SV_TARGET {
    const float2 uv = input.uv;

    int2 texture_size;
    u_Texture0.GetDimensions(texture_size.x, texture_size.y);
    float2 noise_scale = float2(texture_size);
    noise_scale *= (1.0f / SSAO_NOISE_SIZE);

    float3 N = SAMPLE_TEXTURE_2D(u_Texture0, s_pointClampSampler, uv).rgb;
    N = 2.0f * N - 1.0f;

    const float raw_depth = SAMPLE_TEXTURE_2D(u_Texture1, s_pointClampSampler, uv).r;
    const float3 origin = NdcToViewPos(uv, NormalizeDepth(raw_depth), c_invCamProj);

    float3 rvec = float3(SAMPLE_TEXTURE_LEVEL(u_Texture2, s_pointWrapSampler, uv * noise_scale, 0.0f).xy, 0.0f);
    float3 tangent = normalize(rvec - N * dot(rvec, N));
    float3 bitangent = cross(N, tangent);

    float3x3 TBN = float3x3(tangent, bitangent, N);

#if 0
    return mul(float3(0, 0, 1), TBN).b;
#endif

    float occlusion = 0.0;

    for (int i = 0; i < SSAO_KERNEL_SIZE; ++i) {
        // get sample position
        float3 samplePos = mul(c_ssaoKernel[i].xyz, TBN);  // from tangent to view-space
        samplePos = origin.xyz + samplePos * c_ssaoKernalRadius;

        // project sample position (to sample texture) (to get position on screen/texture)
        float4 offset = float4(samplePos, 1.0);
        offset = mul(c_camProj, offset);    // from view to clip-space
        offset /= offset.w;                 // perspective divide
        offset.xy = offset.xy * 0.5 + 0.5;  // transform to range 0.0 - 1.0

        const float depth2 = SAMPLE_TEXTURE_2D(u_Texture1, s_pointClampSampler, offset.xy).r;
        const float3 sampleOcclusionPos = NdcToViewPos(offset.xy, NormalizeDepth(depth2), c_invCamProj);
        const float sample_depth = sampleOcclusionPos.z;

        const float range_check = smoothstep(0.0, 1.0, c_ssaoKernalRadius / abs(origin.z - sample_depth));
        const float increment = sample_depth - samplePos.z >= SSAO_KERNEL_BIAS ? 1.0f : 0.0f;
        occlusion += increment;
        // occlusion += increment * range_check;
    }

    occlusion = 1.0 - (occlusion / float(SSAO_KERNEL_SIZE));
    return occlusion;
}
