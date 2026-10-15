/// File: bloom_downsample.slang
#include "common.slang"
#include "../cbuffer.slang.h"

TEXTURE_2D(u_Texture0, 0)
#define t_BloomInputTexture u_Texture0

RWTexture2D<float4> u_BloomOutputImage : register(u0);

[shader("compute")]
[numthreads(16, 16, 1)]
void cs_main(uint3 dispatch_thread_id : SV_DISPATCHTHREADID) {
    const uint2 output_coord = dispatch_thread_id.xy;

    uint width, height;
    u_BloomOutputImage.GetDimensions(width, height);
    float2 output_image_size = float2(width, height);

    float2 uv = float2(output_coord.x / output_image_size.x,
                       output_coord.y / output_image_size.y);

    uint input_width, input_height;
    TEX_2D(t_BloomInputTexture).GetDimensions(input_width, input_height);
    float x = 1.0f / input_width;
    float y = 1.0f / input_height;
    uv.x += 0.5f * x;
    uv.y += 0.5f * y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = SAMPLE_LEVEL(PASS(t_BloomInputTexture), s_linearClampSampler, float2(uv.x - 2 * x, uv.y + 2 * y), 0).rgb;
    float3 b = SAMPLE_LEVEL(PASS(t_BloomInputTexture), s_linearClampSampler, float2(uv.x, uv.y + 2 * y), 0).rgb;
    float3 c = SAMPLE_LEVEL(PASS(t_BloomInputTexture), s_linearClampSampler, float2(uv.x + 2 * x, uv.y + 2 * y), 0).rgb;

    float3 d = SAMPLE_LEVEL(PASS(t_BloomInputTexture), s_linearClampSampler, float2(uv.x - 2 * x, uv.y), 0).rgb;
    float3 e = SAMPLE_LEVEL(PASS(t_BloomInputTexture), s_linearClampSampler, float2(uv.x, uv.y), 0).rgb;
    float3 f = SAMPLE_LEVEL(PASS(t_BloomInputTexture), s_linearClampSampler, float2(uv.x + 2 * x, uv.y), 0).rgb;

    float3 g = SAMPLE_LEVEL(PASS(t_BloomInputTexture), s_linearClampSampler, float2(uv.x - 2 * x, uv.y - 2 * y), 0).rgb;
    float3 h = SAMPLE_LEVEL(PASS(t_BloomInputTexture), s_linearClampSampler, float2(uv.x, uv.y - 2 * y), 0).rgb;
    float3 i = SAMPLE_LEVEL(PASS(t_BloomInputTexture), s_linearClampSampler, float2(uv.x + 2 * x, uv.y - 2 * y), 0).rgb;

    float3 j = SAMPLE_LEVEL(PASS(t_BloomInputTexture), s_linearClampSampler, float2(uv.x - x, uv.y + y), 0).rgb;
    float3 k = SAMPLE_LEVEL(PASS(t_BloomInputTexture), s_linearClampSampler, float2(uv.x + x, uv.y + y), 0).rgb;
    float3 l = SAMPLE_LEVEL(PASS(t_BloomInputTexture), s_linearClampSampler, float2(uv.x - x, uv.y - y), 0).rgb;
    float3 m = SAMPLE_LEVEL(PASS(t_BloomInputTexture), s_linearClampSampler, float2(uv.x + x, uv.y - y), 0).rgb;

    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5

    float3 final_color = e * 0.125f;
    final_color += (a + c + g + i) * 0.03125f;
    final_color += (b + d + f + h) * 0.0625f;
    final_color += (j + k + l + m) * 0.125f;

    final_color = max(final_color, 0.000f);
    u_BloomOutputImage[output_coord] = float4(final_color, 1.0f);
}