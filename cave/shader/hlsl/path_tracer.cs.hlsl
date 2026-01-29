/// File: pass_tracer.cs.hlsl
#include "cbuffer.hlsl.h"
#include "shader_defines.hlsl.h"
#include "shader_resource_defines.hlsl.h"

// @NOTE: include this at last
#include "shared_path_tracer.h"

RWTexture2D<float4> u_PathTracerOutputImage : register(u0);

[numthreads(16, 16, 1)] void main(uint3 p_dispatch_thread_id : SV_DISPATCHTHREADID) {
    // random seed
    // [0, width], [0, height]
    const int2 uv = int2(p_dispatch_thread_id.xy);
    // iPixelCoords += tileOffset;
    float2 fPixelCoords = float2(float(uv.x), float(uv.y));
    uint2 dims;
    u_PathTracerOutputImage.GetDimensions(dims.x, dims.y);

    uint seed = uint(uint(uv.x) * uint(1973) + uint(uv.y) * uint(9277) + uint(c_frameIndex) * uint(26699)) | uint(1);

    // [-0.5, 0.5]
    float2 jitter = float2(Random(seed), Random(seed)) - 0.5;

    float3 rayDir;
    {
        // screen position from [-1, 1]
        float2 uvJitter = (fPixelCoords + jitter) / dims;
        float2 screen = 2.0 * uvJitter - 1.0;

        // adjust for aspect ratio
        float2 resolution = float2(float(dims.x), float(dims.y));
        float aspectRatio = resolution.x / resolution.y;
        screen.y /= aspectRatio;
        float halfFov = c_cameraFovDegree;
        float camDistance = tan(halfFov * MY_PI / 180.0);
        rayDir = float3(screen, camDistance);
        float3x3 mat = float3x3(c_cameraRight, c_cameraUp, c_cameraForward);
        rayDir = normalize(mul(rayDir, mat));
    }

    Ray ray;
    ray.origin = c_cameraPosition;
    ray.direction = rayDir;
    ray.invDir = 1.0f / ray.direction;
    ray.t = RAY_T_MAX;

    float3 radiance = RayColor(ray, seed);

    float4 final_color = float4(radiance, 1.0);

#if FORCE_UPDATE == 0
    if (c_sceneDirty == 0) {
        float4 accumulated = u_PathTracerOutputImage[uv];
        float weight = accumulated.a;
        float3 new_color = radiance + weight * accumulated.rgb;
        float new_weight = accumulated.a + 1.0;
        new_color /= new_weight;
        final_color = float4(new_color, new_weight);
    }
#endif

    u_PathTracerOutputImage[uv] = final_color;
}
