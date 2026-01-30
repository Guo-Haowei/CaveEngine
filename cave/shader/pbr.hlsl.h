/// File: pbr.hlsl.h
#ifndef PBR_HLSL_H_INCLUDED
#define PBR_HLSL_H_INCLUDED
#include "shader_defines.hlsl.h"

#if defined(__cplusplus)
#include <cave/core/math/Vector.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
using namespace cave;
using ::cave::math::clamp;
using ::cave::math::cross;
using ::cave::math::max;
using ::cave::math::min;
using ::cave::math::normalize;
#endif

#define MAX_REFLECTION_LOD 4.0

// NDF(n, h, alpha) = alpha^2 / (pi * ((n dot h)^2 * (alpha^2 - 1) + 1)^2)
float DistributionGGX(float NdotH, float p_roughness) {
    float a = p_roughness * p_roughness;
    float a2 = a * a;
    float nom = a2;
    float denom = NdotH * NdotH * (a2 - 1.0f) + 1.0f;
    denom = MY_PI * denom * denom;
    // if roughness = 0, NDF = 0,
    // if roughness = 1, NDF = 1 / pi
    return nom / denom;
}

float GeometrySchlickGGX(float p_n_dot_h, float p_roughness) {
    // note that we use a different k for IBL
    float a = p_roughness;
    float k = (a * a) / 2.0f;

    float nom = p_n_dot_h;
    float denom = p_n_dot_h * (1.0f - k) + k;

    return nom / denom;
}

// GSchlickGGX(n, v, k) = dot(n, v) / (dot(n, v)(1 - k) + k)
// k is a remapping of alpha
float GSchlickGGX(float p_n_dot_h, float p_roughness) {
    float r = (p_roughness + 1.0f);
    float k = (r * r) / 8.0f;

    float nom = p_n_dot_h;
    float denom = p_n_dot_h * (1.0f - k) + k;
    return nom / denom;
}

float GeometrySmith(float p_n_dot_v, float p_n_dot_l, float p_roughness) {
    float ggx2 = GeometrySchlickGGX(p_n_dot_v, p_roughness);
    float ggx1 = GeometrySchlickGGX(p_n_dot_l, p_roughness);

    return ggx1 * ggx2;
}

float3 FresnelSchlick(float p_cos_theta, const float3 p_f0) {
    return p_f0 + (1.0f - p_f0) * pow(1.0f - p_cos_theta, 5.0f);
}

float RadicalInverseVDC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10f;  // / 0x100000000
}

float2 Hammersley(uint i, uint N) {
    return float2(float(i) / float(N), RadicalInverseVDC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness) {
    float a = roughness * roughness;

    float phi = 2.0f * MY_PI * Xi.x;
    float cos_theta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
    float sin_theta = sqrt(1.0f - cos_theta * cos_theta);

    // from spherical coordinates to cartesian coordinates - halfway vector
    float3 H;
    H.x = cos(phi) * sin_theta;
    H.y = sin(phi) * sin_theta;
    H.z = cos_theta;

    // from tangent-space H vector to world-space sample vector
    float3 up = abs(N.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);

    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

float3 FresnelSchlickRoughness(float p_cos_theta, float3 F0, float p_roughness) {
    float3 zero = float3(0.0f, 0.0f, 0.0f);
    float3 tmp = float3(1.0f, 1.0f, 1.0f) - p_roughness;
    return F0 + (max(tmp - F0, zero)) * pow(1.0f - p_cos_theta, 5.0f);
}

float2 IntegrateBRDF(float NdotV, float roughness) {
    float3 V;
    V.x = sqrt(1.0f - NdotV * NdotV);
    V.y = 0.0f;
    V.z = NdotV;

    float A = 0.0f;
    float B = 0.0f;

    float3 N = float3(0.0f, 0.0f, 1.0f);

    const uint SAMPLE_COUNT = 1024u;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
        // generates a sample vector that's biased towards the
        // preferred alignment direction (importance sampling).
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, roughness);
        float3 L = normalize(2.0f * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0f);
        float NdotH = max(H.z, 0.0f);
        float VdotH = max(dot(V, H), 0.0f);

        if (NdotL > 0.0f) {
            float G = GeometrySmith(NdotV, NdotL, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0f - VdotH, 5.0f);

            A += (1.0f - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return float2(A, B);
}

float3 lighting(float3 N,
                float3 L,
                float3 V,
                float3 radiance,
                float3 F0,
                float roughness,
                float metallic,
                float3 p_base_color) {

    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    const float3 H = normalize(V + L);
    const float NdotL = max(dot(N, L), 0.0f);
    const float NdotH = max(dot(N, H), 0.0f);
    const float NdotV = max(dot(N, V), 0.0f);

    // direct cook-torrance brdf
    const float NDF = DistributionGGX(NdotH, roughness);
    const float G = GeometrySmith(NdotV, NdotL, roughness);
    const float3 F = FresnelSchlick(clamp(dot(H, V), 0.0f, 1.0f), F0);

    const float3 nom = NDF * G * F;
    float denom = 4 * NdotV * NdotL;

    float3 specular = nom / max(denom, 0.001f);

    const float3 kS = F;
    const float3 kD = (1.0f - metallic) * (float3(1.0f, 1.0f, 1.0f) - kS);

    float3 direct_lighting = (kD * p_base_color / MY_PI + specular) * radiance * NdotL;

    return direct_lighting;
}

#endif