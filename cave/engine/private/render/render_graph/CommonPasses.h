#pragma once
#include "RenderGraph.h"

namespace cave::render {

struct DepthPrepassOutput {
    RGTextureId depth{};
};

struct GbufferOutput {
    RGTextureId color0{};
    RGTextureId color1{};
    RGTextureId color2{};
};

struct PostProcessInput {
    RGTextureId lighting{};
    RGTextureId outline{};
    RGTextureId bloom{};
    GpuTextureId color_attachment{};
};

struct PostProcessOutput {
    RGTextureId processed{};
    RGTextureId ds{};
};

struct LightingInput {
    RGTextureId color0{};
    RGTextureId color1{};
    RGTextureId color2{};
    RGTextureId depth{};
    RGTextureId ssao{};
    RGTextureId shadow{};
    RGTextureId ibl_diffuse{};
    RGTextureId ibl_prefiltered{};
    RGTextureId brdf{};
    RGTextureId ltc1{};
    RGTextureId ltc2{};
};

struct LightingOutput {
    RGTextureId lighting{};
};

struct ForwardInput {
    RGTextureId skybox{};
    RGTextureId shadow{};
    RGTextureId ibl_diffuse{};
    RGTextureId ibl_prefiltered{};
    RGTextureId brdf{};
    RGTextureId ltc1{};
    RGTextureId ltc2{};

    // output
    RGTextureId depth{};
    RGTextureId lighting{};
};

struct ForwardOutput {
};

struct HighlightInput {
    RGTextureId stencil{};
};

struct HighlightOutput {
    RGTextureId outline{};
};

struct TwoDInput {
    GpuTextureId color_attachment{};
};

class RenderGraphBuilderExt : public RenderGraph {
public:
    [[nodiscard]]
    DepthPrepassOutput addDepthPrepass();

    [[nodiscard]]
    GbufferOutput addGbufferPass(const DepthPrepassOutput& in);

    [[nodiscard]]
    LightingOutput addLightingPass(const LightingInput& in);

    [[nodiscard]]
    ForwardOutput addForwardPass(const ForwardInput& in);

    [[nodiscard]]
    HighlightOutput addHighlightPass(const HighlightInput& in);

    PostProcessOutput addPostProcessPass(const PostProcessInput& p_in);
    // void AddVoxelizationPass();
    // void AddBloomPass();

    void add2dPass(const TwoDInput& in);
};

}  // namespace cave::render
