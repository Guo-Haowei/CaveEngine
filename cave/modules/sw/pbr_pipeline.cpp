#include "pbr_pipeline.h"

#include "engine/private/runtime/assets/ImageAsset.h"

#include "pbr.hlsl.h"

namespace cave {

using namespace math;

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

static Vec3f Sample(ImageAsset* p_image, Vec2f uv) {
    const int x = static_cast<int>(uv.x * p_image->width);
    const int y = static_cast<int>(uv.y * p_image->height);
    if (x < 0 || x >= p_image->width || y < 0 || y >= p_image->height) {
        return Vec3f::Zero;
    }

    const int index = y * p_image->width + x;
    Vec3f color;
    color.r = p_image->buffer[index * 4 + 0];
    color.g = p_image->buffer[index * 4 + 1];
    color.b = p_image->buffer[index * 4 + 2];
    if (p_image->color_space == ImageAsset::ColorSpace::SRGB) {
        color.r = SrgbToLinear(color.r / 255.5f);
        color.g = SrgbToLinear(color.g / 255.5f);
        color.b = SrgbToLinear(color.b / 255.5f);
    }

    return color;
}

Vec3f PbrPipeline::ProcessFragment(const VSOutput& input) {
    Vec3f base_color;
    if (material_cb.c_hasBaseColorMap && material_cb.c_baseColorMapHandle) {
        ImageAsset* image = (ImageAsset*)material_cb.c_baseColorMapHandle;
        base_color = Sample(image, input.uv);
    } else {
        base_color = material_cb.c_baseColor.xyz;
    }

    Vec3f final_color = ComputeLighting(base_color,
                                        Vec3f::Zero,
                                        input.normal.xyz,
                                        material_cb.c_metallic,
                                        material_cb.c_roughness,
                                        material_cb.c_emissivePower);

    return final_color;
}

Vec3f PbrPipeline::ComputeLighting(Vec3f base_color,
                                   Vec3f world_position,
                                   Vec3f N,
                                   float metallic,
                                   float roughness,
                                   float emissive) {
    if (emissive > 0.0) {
        return Vec3f(emissive * base_color);
    }

    const Vec3f V = normalize(per_frame_cb.c_cameraPosition - world_position);
    const float NdotV = cave::max(dot(N, V), 0.0f);
    Vec3f R = glm::reflect(-V, N);

    Vec3f Lo = Vec3f(0.0f);
    Vec3f F0 = cave::lerp(Vec3f(0.04f), base_color, metallic);

    const Vec3f radiance = Vec3f(LIGHT_INTENSITY);
    Vec3f delta = -world_position + LIGHT_POS;
    Vec3f L = normalize(delta);
    const Vec3f H = normalize(V + L);
    Vec3f direct_lighting = lighting(N, L, V, radiance, F0, roughness, metallic, base_color);
    Lo += direct_lighting;

    Vec3f F = FresnelSchlickRoughness(NdotV, F0, roughness);
    Vec3f kS = F;
    Vec3f kD = 1.0f - kS;
    kD *= 1.0f - metallic;

#if 1
    Vec3f diffuse = AMBIENT_COLOR.xyz;

    const float ao = 1.0;
    Vec3f ambient = (kD * diffuse) * ao;
#endif

    Vec3f final_color = Lo + ambient;
    return final_color;
}

}  // namespace cave
