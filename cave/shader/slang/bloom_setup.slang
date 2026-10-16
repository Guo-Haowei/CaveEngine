/// File: bloom_setup.slang
#include "../cbuffer.slang.h"
#include "common.slang"

#if !defined(__TARGET_D3D12__)
RWTexture2D<float4> u_BloomOutputImage : register(u0);
#endif

TEXTURE_2D(u_Texture0, 0);

#define t_TextureLighting u_Texture0

float rgb_to_luma(float3 rgb) {
    return dot(max(rgb, 0.0), float3(0.2126, 0.7152, 0.0722));
}

[shader("compute")]
[numthreads(16, 16, 1)]
void cs_main(uint3 dispatch_thread_id : SV_DISPATCHTHREADID) {
    uint2 output_coord = dispatch_thread_id.xy;

    uint width, height;

#if defined(__TARGET_D3D12__)
    RWTexture2D<float4> output = RW_TEX_2D(c_output_uav0);
#else
    RWTexture2D<float4> output = u_BloomOutputImage;
#endif

    output.GetDimensions(width, height);
    float2 output_image_size = float2(width, height);
    float2 uv = float2(output_coord.x / output_image_size.x,
                       output_coord.y / output_image_size.y);

    float3 color = SAMPLE_LEVEL(PASS(t_TextureLighting), s_linearClampSampler, uv, 0).rgb;
    color = max(color, 0.0);

    const float luma = rgb_to_luma(color);
    const float contribution = max(luma - c_bloom_threshold, 0.0) / max(luma, 1e-5);

    output[output_coord] = float4(color * contribution, 1.0f);
}