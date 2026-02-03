#pragma once
#include "engine/private/render/render_graph/RGTextureId.h"

namespace cave::render {

struct RenderOptions;
class RenderGraph;

class ShadowFeature {
public:
    struct Outputs {
        RGTextureId shadow{};
    };

    [[nodiscard]] Outputs Build(RenderGraph& p_graph, const RenderOptions& p_plan);

private:
    // @TODO: reuse shadow map, shadow atlas, etc
};

}  // namespace cave::render
