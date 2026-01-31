#pragma once
#include "RenderGraphBuilder.h"

namespace cave::render {

struct FinalTarget {
    std::shared_ptr<GpuTexture> color;
    std::shared_ptr<GpuTexture> depth;
};

struct ShadowOutput {
    RGTextureId shadow{};
};

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
    const FinalTarget* target{ nullptr };
};

struct LightingOutput {
    RGTextureId lighting{};
};

class RenderGraphBuilderExt : public RenderGraphBuilder {
public:
    // @TODO: create 2D
    [[nodiscard]] auto Create3D(RenderGraphBuilderConfig& p_config,
                                const FinalTarget& p_target) -> Result<std::shared_ptr<RenderGraph>>;
    //[[nodiscard]] auto CreatePathTracer(RenderGraphBuilderConfig& p_config) -> Result<std::shared_ptr<RenderGraph>>;

private:
    [[nodiscard]] ShadowOutput AddShadowPass();
    [[nodiscard]] DepthPrepassOutput AddDepthPrepass();
    [[nodiscard]] GbufferOutput AddGbufferPass(const DepthPrepassOutput& p_in);
    [[nodiscard]] LightingOutput AddLightingPass(const LightingInput& p_in);
    [[nodiscard]] SsaoOutput AddSsaoPass(const SsaoInput& p_in);
    [[nodiscard]] PostProcessOutput AddPostProcessPass(const PostProcessInput& p_in);
    // void AddForwardPass();
    // void AddHighlightPass();
    // void AddVoxelizationPass();
    // void AddBloomPass();
    // void AddGenerateSkylightPass();

    void AddPathTracerPass();
    void AddPathTracerTonePass();
};

}  // namespace cave::render
