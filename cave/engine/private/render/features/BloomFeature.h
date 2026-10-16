#pragma once
#include "engine/private/render/render_graph/RGTextureId.h"

namespace cave::render {

struct RenderOptions;
class RenderGraph;

class BloomFeature {
public:
    struct Inputs {
        RGDependencyId dependency;

        RGTextureId color;
    };

    struct Outputs {
        RGDependencyId dependency;

        RGTextureId bloom;
    };

    [[nodiscard]] Outputs build(RenderGraph& graph,
                                const Inputs& inputs,
                                const RenderOptions& plan);
};

}  // namespace cave::render
