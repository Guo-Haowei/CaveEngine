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

    [[nodiscard]] Outputs Build(RenderGraph& graph, const RenderOptions& plan);

private:
    // @TODO: reuse shadow map, shadow atlas, etc
};

}  // namespace cave::render
