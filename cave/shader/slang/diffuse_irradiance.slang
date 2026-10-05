/// File: diffuse_irradiance.slang
#include "../cbuffer.hlsl.h"
#include "../hlsl/input_output.hlsl"
#include "common.slang"

#define SAMPLE_STEP 0.025f

TEXTURE_CUBE(u_Texture0, t0);

[shader("fragment")]
float4 ps_main(VS_OUTPUT_POSITION input)
    : SV_TARGET {
    float3 N = normalize(input.world_position);
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = cross(up, N);
    up = cross(N, right);

    float3 irradiance = float3(0.0, 0.0, 0.0);
    float samples = 0.0;

    for (float phi = 0.0; phi < 2.0 * MY_PI; phi += SAMPLE_STEP) {
        for (float theta = 0.0; theta < 0.5 * MY_PI; theta += SAMPLE_STEP) {
            float xdir = sin(theta) * cos(phi);
            float ydir = sin(theta) * sin(phi);
            float zdir = cos(theta);
            float3 sample_dir = xdir * right + ydir * up + zdir * N;
            irradiance += SAMPLE_TEXTURE_LOD(u_Texture0, s_cubemapClampSampler, sample_dir, 0.0).rgb * cos(theta) * sin(theta);
            samples += 1.0;
        }
    }

    irradiance = MY_PI * irradiance * (1.0 / samples);

    return float4(irradiance, 1.0);
}