#pragma once
#include "RenderGraphBuilder.h"

namespace cave::render {

struct FinalTarget {
    std::shared_ptr<GpuTexture> color;
    std::shared_ptr<GpuTexture> depth;
};

struct ShadowOutput {
    RGTextureHandle shadow{};
};

struct DepthPrepassOutput {
    RGTextureHandle depth{};
};

struct GbufferOutput {
    RGTextureHandle color0{};
    RGTextureHandle color1{};
    RGTextureHandle color2{};
};

struct PostProcessInput {
    RGTextureHandle lighting{};
    RGTextureHandle outline{};
    RGTextureHandle bloom{};
};

struct PostProcessOutput {
    RGTextureHandle processed{};
    RGTextureHandle ds{};
};

struct SsaoInput {
    RGTextureHandle depth{};
    RGTextureHandle normal{};
};

struct SsaoOutput {
    RGTextureHandle processed{};
};

struct LightingInput {
    RGTextureHandle color0{};
    RGTextureHandle color1{};
    RGTextureHandle color2{};
    RGTextureHandle depth{};
    RGTextureHandle ssao{};
    RGTextureHandle shadow{};
    RGTextureHandle ibl_diffuse{};
    RGTextureHandle ibl_prefiltered{};
    const FinalTarget* target{ nullptr };
};

struct LightingOutput {
    RGTextureHandle lighting{};
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
