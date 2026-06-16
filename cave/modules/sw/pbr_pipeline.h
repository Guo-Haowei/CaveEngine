#pragma once
#include "sw_renderer.h"

#include "engine/private/core/math/geomath.h"

namespace cave {

#include "cbuffer.hlsl.h"

// @TODO: refactor
constexpr math::Vec3f LIGHT_POS{ -5, 6, 3 };
constexpr math::Vec4f AMBIENT_COLOR{ .06f, .06f, .06f, 1.0f };
constexpr float LIGHT_INTENSITY = 3.0f;

class PbrPipeline : public SwPipeline {
public:
    PbrPipeline()
        : SwPipeline(VARYING_UV | VARYING_NORMAL | VARYING_WORLD_POSITION) {}

    virtual VSOutput ProcessVertex(const VSInput& input) override;

    virtual math::Vec3f ProcessFragment(const VSOutput& input) override;

    math::Vec3f ComputeLighting(math::Vec3f base_color,
                                math::Vec3f world_position,
                                math::Vec3f N,
                                float metallic,
                                float roughness,
                                float emissive);

    PerBatchConstantBuffer per_batch_cb;
    PerFrameConstantBuffer per_frame_cb;
    MaterialConstantBuffer material_cb;
};

}  // namespace cave
