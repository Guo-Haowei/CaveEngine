/// File: ssao.ps.hlsl
#include "cbuffer.hlsl.h"
#include "common.hlsl.h"
#include "hlsl/input_output.hlsl"
#include "sampler.hlsl.h"

Texture2D t_GbufferNormalMap : register(t0);
Texture2D t_GbufferDepth : register(t1);
Texture2D t_NoiseTexture : register(t2);

// @TODO: fix HARD CODE
#define SSAO_KERNEL_BIAS 0.025f

float main(VS_OUTPUT_UV input)
    : SV_TARGET {
    const float2 uv = input.uv;

    int2 texture_size;
    t_GbufferNormalMap.GetDimensions(texture_size.x, texture_size.y);
    float2 noise_scale = float2(texture_size);
    noise_scale *= (1.0f / SSAO_NOISE_SIZE);

    float3 N = t_GbufferNormalMap.Sample(s_pointClampSampler, uv).rgb;
    N = 2.0f * N - 1.0f;

    // reconstruct view position
    // https://stackoverflow.com/questions/11277501/how-to-recover-view-space-position-given-view-space-depth-value-and-ndc-xy
    const float depth = t_GbufferDepth.Sample(s_pointClampSampler, uv).r;
    const float3 origin = NdcToViewPos(uv, depth);

    float3 rvec = float3(t_NoiseTexture.SampleLevel(s_pointWrapSampler, uv * noise_scale, 0.0f).xy, 0.0f);
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

        const float depth2 = t_GbufferDepth.Sample(s_pointClampSampler, offset.xy).r;
        const float3 sampleOcclusionPos = NdcToViewPos(offset.xy, depth2);
        const float sample_depth = sampleOcclusionPos.z;

        const float range_check = smoothstep(0.0, 1.0, c_ssaoKernalRadius / abs(origin.z - sample_depth));
        const float increment = sample_depth - samplePos.z >= SSAO_KERNEL_BIAS ? 1.0f : 0.0f;
        occlusion += increment;
        // occlusion += increment * range_check;
    }

    occlusion = 1.0 - (occlusion / float(SSAO_KERNEL_SIZE));
    return occlusion;
}
