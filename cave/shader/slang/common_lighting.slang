#include "../pbr.hlsl.h"
#include "common_shadow.slang"

static const uint NUM_POINT_SHADOW_SAMPLES = 20;
static const float LUT_SIZE = 64.0;
static const float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;
static const float LUT_BIAS = 0.5 / LUT_SIZE;

static float3 POINT_LIGHT_SHADOW_SAMPLE_OFFSET[NUM_POINT_SHADOW_SAMPLES] = {
    float3(1, 1, 1),
    float3(1, -1, 1),
    float3(-1, -1, 1),
    float3(-1, 1, 1),
    float3(1, 1, -1),
    float3(1, -1, -1),
    float3(-1, -1, -1),
    float3(-1, 1, -1),
    float3(1, 1, 0),
    float3(1, -1, 0),
    float3(-1, -1, 0),
    float3(-1, 1, 0),
    float3(1, 0, 1),
    float3(-1, 0, 1),
    float3(1, 0, -1),
    float3(-1, 0, -1),
    float3(0, 1, 1),
    float3(0, -1, 1),
    float3(0, -1, -1),
    float3(0, 1, -1),
};

float point_shadow_calculation(Light p_light, float3 p_frag_pos, float3 p_eye) {
    float3 light_position = p_light.position;
    float light_far = p_light.max_distance;

    float3 frag_to_light = p_frag_pos - light_position;
    float current_depth = length(frag_to_light);

    float bias = 0.01;

    float view_distance = length(p_eye - p_frag_pos);
    float disk_radius = (1.0 + (view_distance / light_far)) / 300.0;

    float shadow = 0.0;

#if 0
    float4 uv = float4(frag_to_light, (float)p_light.shadow_map_index);
    float closest_depth = TEXTURE_CUBE_ARRAY(PointShadowArray).SampleLevel(s_shadowSampler, uv, 0).r;
    closest_depth *= light_far;
    if (current_depth - bias > closest_depth) {
        shadow += 1.0;
    }
    ///
    for (int i = 0; i < NUM_POINT_SHADOW_SAMPLES; ++i) {
        float4 uv = float4(frag_to_light + POINT_LIGHT_SHADOW_SAMPLE_OFFSET[i] * disk_radius, (float)p_light.shadow_map_index);
        float closest_depth = TEXTURE_CUBE_ARRAY(pointShadowArray).SampleLevel(s_shadowSampler, uv, 0).r;
        closest_depth *= light_far;
        if (current_depth - bias > closest_depth) {
            shadow += 1.0;
        }
    }
    shadow /= float(NUM_POINT_SHADOW_SAMPLES);
#endif

    return shadow;
}

// Rectangle light edge integration helper
float3 IntegrateEdgeVec(float3 v1, float3 v2) {
    // Using fitting result for calculating acos()
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206 * y) * y;
    float b = 3.4175940 + (4.1616724 + y) * y;
    float v = a / b;

    float theta_sintheta = (x > 0.0) ? v : 0.5 * rsqrt(max(1.0 - x * x, 1e-7)) - v;

    return cross(v1, v2) * theta_sintheta;
}

float3 LTC_Evaluate(float3 N, float3 V, float3 P, float3x3 Minv, float4 points[4]) {
    // Construct orthonormal basis around N
    float3 T1 = normalize(V - N * dot(V, N));
    float3 T2 = cross(N, T1);

    // Rotate area light in (T1, T2, N) basis
    // GLSL's transpose(mat3(T1, T2, N)) corresponds to row-constructor float3x3(T1, T2, N) in Slang
    float3x3 R = float3x3(T1, T2, N);
    Minv = mul(Minv, R);

    // Polygon (allocate 4 vertices for clipping)
    float3 L[4];

    // Transform polygon from LTC back to origin (cosine weighted)
    float3 point0 = points[0].xyz;
    float3 point1 = points[1].xyz;
    float3 point2 = points[2].xyz;
    float3 point3 = points[3].xyz;

    L[0] = mul(Minv, point0 - P);
    L[1] = mul(Minv, point1 - P);
    L[2] = mul(Minv, point2 - P);
    L[3] = mul(Minv, point3 - P);

    // Use tabulated horizon-clipped sphere
    // Check if the shading point is behind the light
    float3 dir = point0 - P; // LTC space
    float3 lightNormal = cross(point1 - point0, point3 - point0);
    bool behind = (dot(dir, lightNormal) < 0.0);

    // Cosine weighted space
    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);

    // Integrate
    float3 vsum = float3(0.0, 0.0, 0.0);
    vsum += IntegrateEdgeVec(L[0], L[1]);
    vsum += IntegrateEdgeVec(L[1], L[2]);
    vsum += IntegrateEdgeVec(L[2], L[3]);
    vsum += IntegrateEdgeVec(L[3], L[0]);

    // Form factor of the polygon in direction vsum
    float len = length(vsum);

    float z = vsum.z / len;
    if (behind)
        z = -z;

    float2 texcoord = float2(z * 0.5 + 0.5, len); // range [0, 1]
    texcoord = texcoord * LUT_SCALE + LUT_BIAS;

    // Fetch the form factor for horizon clipping
    float scale = SAMPLE_TEXTURE_LEVEL(t_LTC2, s_linearClampSampler, texcoord, 0).w;

    float sum = len * scale;
    if (!behind)
        sum = 0.0;

    // Outgoing radiance (solid angle) for the entire polygon
    return float3(sum, sum, sum);
}

float3 area_light(float3x3 Minv, float3 N, float3 V, float3 world_position, float4 p_t2, float3 p_specular, float3 p_diffuse, float4 p_points[4]) {
    float3x3 identity = float3x3(
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0
    );

    float3 diffuse = LTC_Evaluate(N, V, world_position, identity, p_points);
    float3 specular = LTC_Evaluate(N, V, world_position, Minv, p_points);

    float3 kS = p_specular;
    float3 kD = p_diffuse;

    specular *= kS * p_t2.x + (1.0 - kS) * p_t2.y;
    return (specular + kD * diffuse);
}

float3 compute_lighting(
#if defined(__TARGET_GLSL__)
                        Sampler2D shadow_map,
#else
                        Texture2D shadow_map,
#endif
                        float3 base_color,
                        float3 world_position,
                        float3 N,
                        float metallic,
                        float roughness,
                        float emissive) {
    if (emissive > 0.0) {
        return emissive * base_color;
    }

    float3 V = normalize(c_cameraPosition - world_position);
    float NdotV = clamp(dot(N, V), 0.0, 1.0);
    float3 R = reflect(-V, N);

    float3 Lo = float3(0.0, 0.0, 0.0);
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), base_color, metallic);

    // LTC Lookup for Area Lighting
    float2 uv = float2(roughness, sqrt(1.0 - NdotV));
    uv = uv * LUT_SCALE + LUT_BIAS;

    float4 t1 = SAMPLE_TEXTURE_2D(t_LTC1, s_linearClampSampler, uv);
    float4 t2 = SAMPLE_TEXTURE_2D(t_LTC2, s_linearClampSampler, uv);

    // GLSL mat3(col0, col1, col2) translates to HLSL/Slang float3x3 row construction
    float3x3 Minv = float3x3(
        t1.x, 0.0, t1.z,
        0.0,  1.0, 0.0,
        t1.y, 0.0, t1.w
    );

    for (int light_idx = 0; light_idx < MAX_LIGHT_COUNT; ++light_idx) {
        if (light_idx >= c_lightCount) {
            break;
        }

        Light light = c_lights[light_idx];
        int light_type = c_lights[light_idx].type;
        float3 direct_lighting = float3(0.0, 0.0, 0.0);
        float shadow = 0.0;
        float3 radiance = light.color;
        switch (light.type) {
            case LIGHT_TYPE_INFINITE: {
                float3 L = light.position;
                float atten = 1.0;
                const float3 H = normalize(V + L);
                direct_lighting = atten * lighting(N, L, V, radiance, F0, roughness, metallic, base_color);
                if (light.cast_shadow == 1) {
                    const float NdotL = max(dot(N, L), 0.0);
                    shadow = shadowTest(shadow_map, light, world_position, NdotL);
                    direct_lighting *= (1.0 - shadow);
                }
            } break;
            case LIGHT_TYPE_POINT: {
                float3 delta = -world_position + light.position;
                float dist = length(delta);
                float atten = (light.atten_constant + light.atten_linear * dist +
                               light.atten_quadratic * (dist * dist));
                atten = 1.0 / atten;

                if (atten > 0.01) {
                    float3 L = normalize(delta);
                    float3 H = normalize(V + L);
                    direct_lighting = atten * lighting(N, L, V, radiance, F0, roughness, metallic, base_color);
#if 0
                    if (light.cast_shadow == 1) {
                        shadow = point_shadow_calculation(light, world_position, c_cameraPosition);
                    }
#endif
                }
            } break;
            case LIGHT_TYPE_AREA: {
                direct_lighting = radiance * area_light(Minv, N, V, world_position, t2, F0, base_color, light.points);
            } break;
            default:
                break;
        }
        Lo += (1.0 - shadow) * direct_lighting;
    }

    // IBL
    float3 F = FresnelSchlickRoughness(NdotV, F0, roughness);
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    float3 irradiance = float3(0.0, 0.0, 0.0);
    float3 diffuse = float3(0.0, 0.0, 0.0);
    float3 specular = float3(0.0, 0.0, 0.0);
    if (c_iblEnabled != 0) {
        irradiance = SAMPLE_TEXTURE_2D(t_DiffuseIrradiance, s_cubemapClampSampler, N).rgb;
        diffuse = irradiance * base_color.rgb;

        float3 prefilteredColor = SAMPLE_TEXTURE_LEVEL(t_Prefiltered, s_cubemapClampLodSampler, R, roughness * MAX_REFLECTION_LOD).rgb;
        float2 brdf_uv = float2(NdotV, roughness);
        float2 brdf = SAMPLE_TEXTURE_2D(t_BrdfLut, s_linearClampSampler, brdf_uv).rg;
        specular = prefilteredColor * (F * brdf.x + brdf.y);
    }

    const float ao = 1.0;
    float3 ambient = (kD * diffuse + specular) * ao;

    return Lo + ambient;
}