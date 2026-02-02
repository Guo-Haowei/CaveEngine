#pragma once
#include "RenderGraphBuilder.h"

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
};

struct PostProcessOutput {
    RGTextureId processed{};
    RGTextureId ds{};
};

struct SsaoInput {
    RGTextureId depth{};
    RGTextureId normal{};
};

struct SsaoOutput {
    RGTextureId processed{};
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

class RenderGraphBuilderExt : public RenderGraphBuilder {
public:
    //[[nodiscard]] auto CreatePathTracer(RenderGraphBuilderConfig& p_config) -> Result<std::shared_ptr<RenderGraph>>;

    [[nodiscard]] DepthPrepassOutput AddDepthPrepass();
    [[nodiscard]] GbufferOutput AddGbufferPass(const DepthPrepassOutput& p_in);
    [[nodiscard]] LightingOutput AddLightingPass(const LightingInput& p_in);
    [[nodiscard]] SsaoOutput AddSsaoPass(const SsaoInput& p_in);
    [[nodiscard]] PostProcessOutput AddPostProcessPass(const PostProcessInput& p_in);
    [[nodiscard]] ForwardOutput AddForwardPass(const ForwardInput& p_in);
    [[nodiscard]] HighlightOutput AddHighlightPass(const HighlightInput& p_in);
    // void AddVoxelizationPass();
    // void AddBloomPass();

    void AddPathTracerPass();
    void AddPathTracerTonePass();
};

}  // namespace cave::render
