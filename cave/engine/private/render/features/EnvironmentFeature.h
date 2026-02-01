#pragma once
#include "engine/private/render/render_graph/RGTextureId.h"

namespace cave::render {

struct FramePlan;
class RenderGraphBuilder;

class EnvironmentFeature {
public:
    struct Outputs {
        RGTextureId ibl_diffuse{};
        RGTextureId ibl_prefiltered{};
    };

    [[nodiscard]] Outputs Build(RenderGraphBuilder& p_builder, const FramePlan& p_plan);
};

}  // namespace cave::render
