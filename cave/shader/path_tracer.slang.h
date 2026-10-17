/// File: .slang.h
#if defined(__cplusplus)
#include <cave/core/math/Vec.h>

#include <cmath>

namespace cave {
#include "shader_defines.slang.h"
#include "path_tracer_defines.slang.h"

#define inout
#define in
using uint = unsigned int;
using std::cos;
using std::sin;
using std::sqrt;

extern GpuPtVertex GlobalPtVertices[];
extern GpuPtIndex GlobalPtIndices[];
extern GpuPtBvh GlobalPtBvhs[];
extern GpuPtMesh GlobalPtMeshes[];
extern GpuPtMaterial GlobalPtMaterials[];
#endif

#define EPSILON       1.1920929e-7
#define MAX_BOUNCE    5
#define RAY_T_MIN     1e-6
#define RAY_T_MAX     999999999.0
#define TRIANGLE_KIND 1
#define SPHERE_KIND   2

// #define FORCE_UPDATE

struct Ray {
    float3 origin;
    float t;
    float3 direction;
    float3 invDir;
};

struct HitResult {
    float2 uv;
    int hitTriangleId;  // @TODO: rename
    int hitMeshId;
};

struct Sphere {
    float3 A;
    float radius;
};

//------------------------------------------------------------------------------
// Random function
//------------------------------------------------------------------------------
// https://blog.demofox.org/2020/05/25/casual-shadertoy-path-tracing-1-basic-camera-diffuse-emissive/
uint WangHash(inout uint seed) {
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}

// random number between 0 and 1
float Random(inout uint p_state) {
    return float(WangHash(p_state)) / 4294967296.0;
}

// random unit vector
float3 RandomUnitVector(inout uint state) {
    float z = Random(state) * 2.0 - 1.0;
    float a = Random(state) * MY_TWO_PI;
    float r = sqrt(1.0 - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return float3(x, y, z);
}

//------------------------------------------------------------------------------
// Common Ray Trace Functions
//------------------------------------------------------------------------------
// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
HitResult HitTriangle(inout Ray p_ray, int p_triangle_id) {
    // P = A + u(B - A) + v(C - A) => O - A = -tD + u(B - A) + v(C - A)
    // -tD + uAB + vAC = AO
    int3 indices = GlobalPtIndices[p_triangle_id].tri;
    float3 A = GlobalPtVertices[indices.x].position;
    float3 B = GlobalPtVertices[indices.y].position;
    float3 C = GlobalPtVertices[indices.z].position;

    float3 AB = B - A;
    float3 AC = C - A;

    float3 P = cross(p_ray.direction, AC);
    float det = dot(AB, P);

    HitResult result;
    result.hitMeshId = -1;
    result.hitTriangleId = -1;
    result.uv = float2(0.0f, 0.0f);
    if (det < EPSILON) {
        return result;
    }

    float invDet = 1.0 / det;
    float3 AO = p_ray.origin - A;

    float3 Q = cross(AO, AB);
    float u = dot(AO, P) * invDet;
    float v = dot(p_ray.direction, Q) * invDet;

    if (u < 0.0f || v < 0.0f || u + v > 1.0f) {
        return result;
    }

    float t = dot(AC, Q) * invDet;
    if (t >= p_ray.t || t < EPSILON) {
        return result;
    }

    p_ray.t = t;
    result.hitTriangleId = p_triangle_id;
    result.uv = float2(u, v);
    return result;
}

// https://medium.com/@bromanz/another-view-on-the-classic-ray-aabb-intersection-algorithm-for-bvh-traversal-41125138b525
bool HitBvh(in Ray p_ray, in float3 p_min, in float3 p_max) {
    float3 t0s = (p_min - p_ray.origin) * p_ray.invDir;
    float3 t1s = (p_max - p_ray.origin) * p_ray.invDir;

    float3 tsmaller = min(t0s, t1s);
    float3 tbigger = max(t0s, t1s);

#if 0
    float tmin = max(RAY_T_MIN, max(tsmaller.x, max(tsmaller.y, tsmaller.z)));
    float tmax = min(RAY_T_MAX, min(tbigger.x, min(tbigger.y, tbigger.z)));
#else
    float tmin = max(tsmaller.x, max(tsmaller.y, tsmaller.z));
    float tmax = min(tbigger.x, min(tbigger.y, tbigger.z));
#endif

    return (tmin < tmax) && (p_ray.t > tmin);
}

// @TODO: hit result
HitResult HitScene(inout Ray p_ray) {
    HitResult res;
    res.hitMeshId = -1;
    res.hitTriangleId = -1;
    res.uv = float2(0.0f, 0.0f);

    // check if it hits all the objects
    for (int mesh_id = 0; mesh_id < c_pt_object_count; ++mesh_id) {
        GpuPtMesh mesh = GlobalPtMeshes[mesh_id];
        float4x4 inversed = mesh.transform_inv;
        Ray local_ray;
        local_ray.origin = mul(inversed, float4(p_ray.origin, 1.0f)).xyz;
        local_ray.direction = mul(inversed, float4(p_ray.direction, 0.0f)).xyz;
        local_ray.invDir = 1.0f / local_ray.direction;
        local_ray.t = p_ray.t;

        int bvhIndex = mesh.root_bvh_id;
        while (bvhIndex >= 0) {
            GpuPtBvh bvh = GlobalPtBvhs[bvhIndex];
            if (HitBvh(local_ray, bvh.min, bvh.max)) {
                if (bvh.triangleIndex != -1) {
                    HitResult result = HitTriangle(local_ray, bvh.triangleIndex);
                    if (result.hitTriangleId != -1) {
                        res.hitTriangleId = result.hitTriangleId;
                        res.hitMeshId = mesh_id;
                        res.uv = result.uv;
                        p_ray.t = local_ray.t;
                    }
                }
                bvhIndex = bvh.hit_idx;
            } else {
                bvhIndex = bvh.miss_idx;
            }
        }
    }

    return res;
}

float3 RayColor(inout Ray ray, inout uint state) {
    float3 radiance = float3(0, 0, 0);
    // float3 throughput = float3(1, 1, 1);
    float3 throughput = float3(4, 4, 4);

    for (int i = 0; i < MAX_BOUNCE; ++i) {
        // check all objects
        HitResult result = HitScene(ray);
        if (result.hitMeshId >= 0 && result.hitTriangleId >= 0) {
            ray.origin = ray.origin + ray.t * ray.direction;
            ray.t = RAY_T_MAX;

            // calculate normal
            GpuPtMesh mesh = GlobalPtMeshes[result.hitMeshId];

            int3 indices = GlobalPtIndices[result.hitTriangleId].tri;
            float3 n1 = GlobalPtVertices[indices.x].normal;
            float3 n2 = GlobalPtVertices[indices.y].normal;
            float3 n3 = GlobalPtVertices[indices.z].normal;
            float3 n = n1 + result.uv.x * (n2 - n1) + result.uv.y * (n3 - n1);
            n = normalize(mul(mesh.transform, float4(n, 0.0f)).xyz);

            GpuPtMaterial material = GlobalPtMaterials[mesh.material_idx];

            float3 diffuse_color = material.base_color;
            float metallic = material.metallic;
            float roughness = material.roughness;

            float reflect_chance = Random(state) > metallic ? 0.0 : 1.0;

            float3 diffuse_dir = normalize(n + RandomUnitVector(state));
            float3 reflect_dir = reflect(ray.direction, n);
            reflect_dir = normalize(lerp(reflect_dir, diffuse_dir, roughness * roughness));

            ray.direction = normalize(lerp(diffuse_dir, reflect_dir, reflect_chance));
            ray.invDir = 1.0f / ray.direction;

            radiance += material.emissive * throughput;
            throughput *= diffuse_color;
        } else {
            float3 color = SAMPLE_CUBE_LEVEL(PASS(t_Skybox), s_linearClampSampler, ray.direction, 0.0f).rgb;
            radiance += color * throughput;
            break;
        }
    }

    return radiance;
}

#if 0
bool HitSphere(inout Ray ray, in Sphere sphere) {
    float3 oc = ray.origin - sphere.A;
    float a = dot(ray.direction, ray.direction);
    float half_b = dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = half_b * half_b - a * c;

    float t = -half_b - sqrt(discriminant) / a;
    if (discriminant < EPSILON || t >= ray.t || t < EPSILON) {
        return false;
    }

    ray.t = t;
    float3 p = ray.origin + t * ray.direction;
    // ray.hit_normal = normalize(p - sphere.A);
    // ray.material_id = sphere.material_id;
    return true;
}
#endif

#if defined(__cplusplus)
}  // namespace cave
#endif
