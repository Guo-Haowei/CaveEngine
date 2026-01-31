#pragma once
#include "engine/private/render/render_graph/RGTextureId.h"

namespace cave::render {

struct FramePlan;
class RenderGraphBuilder;

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
