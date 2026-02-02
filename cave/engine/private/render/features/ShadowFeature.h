#pragma once
#include "engine/private/render/render_graph/RGTextureId.h"

namespace cave::render {

struct FramePlan;
class RenderGraph;

class ShadowFeature {
public:
    struct Outputs {
        RGTextureId shadow{};
    };

    [[nodiscard]] Outputs Build(RenderGraph& p_graph, const FramePlan& p_plan);

private:
    // @TODO: reuse shadow map, shadow atlas, etc
};

}  // namespace cave::render
