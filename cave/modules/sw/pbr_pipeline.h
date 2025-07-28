#pragma once
#include "sw_renderer.h"

#include "engine/math/geomath.h"

namespace cave {

// @TODO: refactor
constexpr Vector3f CAM_POS{ 0, 0, 2 };
constexpr Vector3f LIGHT_POS{ 0, 4, 4 };
constexpr Vector4f AMBIENT_COLOR{ 0.04f, 0.04f, 0.04f, 1.0f };

class PbrPipeline : public SwPipeline {
public:
    PbrPipeline()
        : SwPipeline(VARYING_UV | VARYING_NORMAL | VARYING_WORLD_POSITION) {}

    virtual VSOutput ProcessVertex(const VSInput& input) override;

    virtual Vector3f ProcessFragment(const VSOutput& input) override;

    Vector3f ComputeLighting(Vector3f base_color,
                             Vector3f world_position,
                             Vector3f N,
                             float metallic,
                             float roughness,
                             float emissive);

public:
    const SwTexture<Color>* m_texture;
    Vector3f c_cameraPosition;

    Matrix4x4f M;
    Matrix4x4f PV;
};

}  // namespace cave
