#pragma once
#include "engine/private/render/render_graph/RGTextureId.h"

namespace cave::render {

class RenderGraphBuilder;

struct FramePlan {
    bool enable_ssao{ false };
    bool enable_bloom{ false };
};

class ShadowFeature {
public:
    struct Outputs {
        RGTextureId shadow{};
    };

    [[nodiscard]] Outputs Build(RenderGraphBuilder& p_builder, const FramePlan& p_plan);

private:
    // @TODO: reuse shadow map, shadow atlas, etc
};

}  // namespace cave::render
