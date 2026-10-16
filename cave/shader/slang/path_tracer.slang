/// File: pass_tracer.slang
#include "../cbuffer.slang.h"
#include "../structured_buffer.slang.h"

#include "common.slang"

#if defined(__TARGET_D3D12__)
#else
RWTexture2D<float4> u_PathTracerOutputImage : register(u0);
StructuredBuffer<GpuPtBvh> GlobalPtBvhs : register(t0);
StructuredBuffer<GpuPtVertex> GlobalPtVertices : register(t1);
StructuredBuffer<GpuPtIndex> GlobalPtIndices : register(t2);
StructuredBuffer<GpuPtMesh> GlobalPtMeshes : register(t3);
StructuredBuffer<GpuPtMaterial> GlobalPtMaterials : register(t4);
#endif

#define GlobalPtBvhs      (StructuredBuffer<GpuPtBvh>(ResourceDescriptorHeap[c_pass_textures[0].x]))
#define GlobalPtVertices  (StructuredBuffer<GpuPtVertex>(ResourceDescriptorHeap[c_pass_textures[1].x]))
#define GlobalPtIndices   (StructuredBuffer<GpuPtIndex>(ResourceDescriptorHeap[c_pass_textures[2].x]))
#define GlobalPtMeshes    (StructuredBuffer<GpuPtMesh>(ResourceDescriptorHeap[c_pass_textures[3].x]))
#define GlobalPtMaterials (StructuredBuffer<GpuPtMaterial>(ResourceDescriptorHeap[c_pass_textures[4].x]))

// @NOTE: include this at last
#include "../shared_path_tracer.h"

[shader("compute")]
[numthreads(16, 16, 1)]
void cs_main(uint3 p_dispatch_thread_id : SV_DISPATCHTHREADID) {
    // random seed
    // [0, width], [0, height]
    const int2 uv = int2(p_dispatch_thread_id.xy);
    // iPixelCoords += tileOffset;
    float2 fPixelCoords = float2(float(uv.x), float(uv.y));
    uint2 dims;

#if defined(__TARGET_D3D12__)
    RWTexture2D<float4> output = RW_TEX_2D(c_output_uav0);
#else
    RWTexture2D<float4> output = u_PathTracerOutputImage;
#endif

    output.GetDimensions(dims.x, dims.y);

    uint seed = uint(uint(uv.x) * uint(1973) + uint(uv.y) * uint(9277) + uint(c_frame_index) * uint(26699)) | uint(1);

    // [-0.5, 0.5]
    float2 jitter = float2(Random(seed), Random(seed)) - 0.5;

    float3 ray_dir;
    {
        // screen position from [-1, 1]
        float2 uvJitter = (fPixelCoords + jitter) / float2(dims);
        float2 screen = 2.0 * uvJitter - 1.0;

#if !defined(__TARGET_OPENGL__)
        screen.y = -screen.y;
#endif

        // adjust for aspect ratio
        float2 resolution = float2(float(dims.x), float(dims.y));
        float aspectRatio = resolution.x / resolution.y;
        screen.x *= aspectRatio;
        float tan_half_fovy = tan(0.5f * c_cam_fovy);
        float3x3 mat = float3x3(c_cam_right.xyz, c_cam_up.xyz, c_cam_forward.xyz);

        ray_dir = float3(screen.x * tan_half_fovy,
                         screen.y * tan_half_fovy,
                         1.0f);
        ray_dir = normalize(mul(ray_dir, mat));
    }

    Ray ray;
    ray.origin = c_cam_pos.xyz;
    ray.direction = ray_dir;
    ray.invDir = 1.0f / ray.direction;
    ray.t = RAY_T_MAX;

    float3 radiance = RayColor(ray, seed);

    float4 final_color = float4(radiance, 1.0);

#if !defined(FORCE_UPDATE)
    if (c_scene_dirty == CAVE_FALSE) {
        float4 accumulated = output[uv];
        float weight = accumulated.a;
        float3 new_color = radiance + weight * accumulated.rgb;
        float new_weight = accumulated.a + 1.0;
        new_color /= new_weight;
        final_color = float4(new_color, new_weight);
    }
#endif

    output[uv] = final_color;
}
