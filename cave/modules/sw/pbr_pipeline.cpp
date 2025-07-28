#include "pbr_pipeline.h"

#include "pbr.hlsl.h"

namespace cave {

VSOutput PbrPipeline::ProcessVertex(const VSInput& input) {
    VSOutput vs_output;
    vs_output.world_position = per_batch_cb.c_worldMatrix * input.position;
    vs_output.position = per_frame_cb.c_camProj *
                         per_frame_cb.c_camView *
                         vs_output.world_position;
    vs_output.normal = per_batch_cb.c_worldMatrix * input.normal;
    vs_output.uv = input.uv;
    return vs_output;
}

FORCE_INLINE float SrgbToLinear(float s) {
    return std::pow((s + 0.055f) / 1.055f, 2.4f);
}

Vector3f PbrPipeline::ProcessFragment(const VSOutput& input) {
    Vector3f base_color;
    // @TODO: use material
    if (m_texture) {
        Color color;
        color = m_texture->sample(input.uv);
        base_color.r = SrgbToLinear(color.r / 255.f);
        base_color.g = SrgbToLinear(color.g / 255.f);
        base_color.b = SrgbToLinear(color.b / 255.f);
    } else {
        base_color = Vector3f(0.5f);
    }

    Vector3f final_color = ComputeLighting(base_color,
                                           Vector3f::Zero,
                                           input.normal.xyz,
                                           0.4f,
                                           0.6f,
                                           0.0f);

    return final_color;
}

Vector3f PbrPipeline::ComputeLighting(Vector3f base_color,
                                      Vector3f world_position,
                                      Vector3f N,
                                      float metallic,
                                      float roughness,
                                      float emissive) {
    if (emissive > 0.0) {
        return Vector3f(emissive * base_color);
    }

    const Vector3f V = normalize(per_frame_cb.c_cameraPosition - world_position);
    const float NdotV = cave::max(dot(N, V), 0.0f);
    Vector3f R = glm::reflect(-V, N);

    Vector3f Lo = Vector3f(0.0f);
    Vector3f F0 = cave::lerp(Vector3f(0.04f), base_color, metallic);

    const Vector3f radiance = Vector3f(4.0f);
    Vector3f delta = -world_position + LIGHT_POS;
    Vector3f L = normalize(delta);
    const Vector3f H = normalize(V + L);
    Vector3f direct_lighting = lighting(N, L, V, radiance, F0, roughness, metallic, base_color);
    Lo += direct_lighting;

    Vector3f F = FresnelSchlickRoughness(NdotV, F0, roughness);
    Vector3f kS = F;
    Vector3f kD = 1.0f - kS;
    kD *= 1.0f - metallic;

#if 1
    Vector3f diffuse = AMBIENT_COLOR.xyz;
    Vector3f specular = Vector3f(0.0f);

    const float ao = 1.0;
    Vector3f ambient = (kD * diffuse + specular) * ao;
#endif

    Vector3f final_color = Lo + ambient;
    return final_color;
}

}  // namespace cave
