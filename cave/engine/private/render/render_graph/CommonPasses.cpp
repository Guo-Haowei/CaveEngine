#include "CommonPasses.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/display/DisplayService.h"

#include "engine/private/algorithm/algorithm.h"
#include "engine/private/render/renderer/FrameData.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/renderer/renderer_misc.h"
#include "engine/private/renderer/sampler.h"
#include "RenderGraphDefines.h"
#include "RenderPass.h"

// @TODO: remove
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {
#include "shader_resource_defines.hlsl.h"
}  // namespace cave

namespace cave::render {

using math::Vec2f;

constexpr const char RG_PASS_2D[] = "p:2d";
constexpr const char RG_PASS_DEPTH_PREPASS[] = "p:early_z";
constexpr const char RG_PASS_GBUFFER[] = "p:gbuffer";
constexpr const char RG_PASS_VOXELIZATION[] = "p:voxelization";
constexpr const char RG_PASS_LIGHTING[] = "p:lighting";
constexpr const char RG_PASS_FORWARD[] = "p:forward";
constexpr const char RG_PASS_BLOOM_SETUP[] = "p:bloom_setup";
constexpr const char RG_PASS_POST_PROCESS[] = "p:post_process";
constexpr const char RG_PASS_OUTLINE[] = "p:outline";

constexpr const char RG_RES_DEPTH_STENCIL[] = "r:depth";
constexpr const char RG_RES_GBUFFER_COLOR0[] = "r:gbuffer0";
constexpr const char RG_RES_GBUFFER_COLOR1[] = "r:gbuffer1";
constexpr const char RG_RES_GBUFFER_COLOR2[] = "r:gbuffer2";
constexpr const char RG_RES_LIGHTING[] = "r:lighting";
constexpr const char RG_RES_POST_PROCESS[] = "r:post_process";
constexpr const char RG_RES_VOXEL_LIGHTING[] = "r:voxel_lighting";
constexpr const char RG_RES_VOXEL_NORMAL[] = "r:voxel_normal";
constexpr const char RG_RES_OUTLINE[] = "r:outline";

extern void DepthPrepassFunc(RenderPassExcutionContext& p_ctx);
extern void GbufferPassFunc(RenderPassExcutionContext& p_ctx);
extern void HighlightPassFunc(RenderPassExcutionContext& p_ctx);
extern void LightingPassFunc(RenderPassExcutionContext& p_ctx);
extern void ForwardPassFunc(RenderPassExcutionContext& p_ctx);
extern void BloomSetupFunc(RenderPassExcutionContext& p_ctx);
extern void BloomDownSampleFunc(RenderPassExcutionContext& p_ctx);
extern void BloomUpSampleFunc(RenderPassExcutionContext& p_ctx);
extern void TonePassFunc(RenderPassExcutionContext& p_ctx);

DepthPrepassOutput RenderGraphBuilderExt::addDepthPrepass() {
    RenderPass& pass = AddPass(RG_PASS_DEPTH_PREPASS);

    DepthPrepassOutput out{
        .depth = CreateTexture({
            RG_RES_DEPTH_STENCIL,
            BuildDefaultTextureDesc(RT_FMT_GBUFFER_DEPTH, AttachmentType::DEPTH_STENCIL_2D),
        }),
    };

    pass.WriteDepth(out.depth, {}, LoadOp::Clear, 0.0f, LoadOp::Clear, STENCIL_FLAG_SKY)
        .SetExecuteFunc(DepthPrepassFunc);

    return out;
}

GbufferOutput RenderGraphBuilderExt::addGbufferPass(const DepthPrepassOutput& p_in) {
    RenderPass& pass = AddPass(RG_PASS_GBUFFER);

    GbufferOutput out{
        .color0 = CreateTexture({
            RG_RES_GBUFFER_COLOR0,
            BuildDefaultTextureDesc(RT_FMT_GBUFFER_BASE_COLOR, AttachmentType::COLOR_2D),
        }),
        .color1 = CreateTexture({
            RG_RES_GBUFFER_COLOR1,
            BuildDefaultTextureDesc(RT_FMT_GBUFFER_NORMAL, AttachmentType::COLOR_2D),
        }),
        .color2 = CreateTexture({
            RG_RES_GBUFFER_COLOR2,
            BuildDefaultTextureDesc(RT_FMT_GBUFFER_MATERIAL, AttachmentType::COLOR_2D),
        }),
    };

    // @TODO: introduce versioning
    pass.ReadDepth(p_in.depth, {}, LoadOp::Load)
        .WriteColor(out.color0, {}, LoadOp::Clear)
        .WriteColor(out.color1, {}, LoadOp::Clear)
        .WriteColor(out.color2, {}, LoadOp::Clear)
        .SetExecuteFunc(GbufferPassFunc);
    return out;
}

LightingOutput RenderGraphBuilderExt::addLightingPass(const LightingInput& p_in) {

    RGTextureId out = CreateTexture({
        RG_RES_LIGHTING,
        BuildDefaultTextureDesc(RT_FMT_LIGHTING, AttachmentType::COLOR_2D),
    });

    RenderPass& pass = AddPass(RG_PASS_LIGHTING);

    pass.Read(ResourceAccess::SRV, p_in.color0)
        .Read(ResourceAccess::SRV, p_in.color1)
        .Read(ResourceAccess::SRV, p_in.color2)
        .Read(ResourceAccess::SRV, p_in.depth)
        .Read(ResourceAccess::SRV, p_in.ssao)
        .Read(ResourceAccess::SRV, p_in.shadow)
        .Read(ResourceAccess::SRV, p_in.ibl_diffuse)
        .Read(ResourceAccess::SRV, p_in.ibl_prefiltered)
        .Read(ResourceAccess::SRV, p_in.brdf)
        .Read(ResourceAccess::SRV, p_in.ltc1)
        .Read(ResourceAccess::SRV, p_in.ltc2)
        .WriteColor(out, {}, LoadOp::Clear)
        .SetExecuteFunc(LightingPassFunc);

    return { out };
}

ForwardOutput RenderGraphBuilderExt::addForwardPass(const ForwardInput& p_in) {
    RenderPass& pass = AddPass(RG_PASS_FORWARD);
    pass.Read(ResourceAccess::SRV, p_in.skybox)
        .Read(ResourceAccess::SRV, p_in.shadow)
        .Read(ResourceAccess::SRV, p_in.ibl_diffuse)
        .Read(ResourceAccess::SRV, p_in.ibl_prefiltered)
        .Read(ResourceAccess::SRV, p_in.brdf)
        .Read(ResourceAccess::SRV, p_in.ltc1)
        .Read(ResourceAccess::SRV, p_in.ltc2)
        .Read(ResourceAccess::NONE, p_in.lighting)  // add dependency
        .ReadDepth(p_in.depth, {}, LoadOp::Load)
        .WriteColor(p_in.lighting, {}, LoadOp::Load)
        .SetExecuteFunc(ForwardPassFunc);

    return ForwardOutput{};
}

HighlightOutput RenderGraphBuilderExt::addHighlightPass(const HighlightInput& p_in) {
    RenderPass& pass = AddPass(RG_PASS_OUTLINE);

    HighlightOutput out = {
        .outline = CreateTexture({
            .debug_name = RG_RES_OUTLINE,
            .resourceDesc = BuildDefaultTextureDesc(RT_FMT_OUTLINE_SELECT, AttachmentType::COLOR_2D),
            .samplerDesc = PointClampSampler(),
        }),
    };

    pass.ReadDepth(p_in.stencil, {}, LoadOp::Load)
        .WriteColor(out.outline, {}, LoadOp::Clear)
        .SetExecuteFunc(HighlightPassFunc);

    return out;
}

PostProcessOutput RenderGraphBuilderExt::addPostProcessPass(const PostProcessInput& in) {
    RenderPass& pass = AddPass(RG_PASS_POST_PROCESS);
    auto desc = BuildDefaultTextureDesc(RT_FMT_TONE,
                                        AttachmentType::COLOR_2D);
    desc.bindFlags |= BIND_SHADER_RESOURCE;

    PostProcessOutput out{
        .processed = ImportTexture({ in.color_attachment }),
    };

    pass.Read(ResourceAccess::SRV, in.lighting)
        .Read(ResourceAccess::SRV, in.outline)
        .Read(ResourceAccess::SRV, in.bloom);

    pass.WriteColor(out.processed, {}, LoadOp::Clear)
        .SetExecuteFunc(TonePassFunc);

    return out;
}

extern void Pass2DDrawFunc(RenderPassExcutionContext& p_ctx);

void RenderGraphBuilderExt::add2dPass(const TwoDInput& in) {
    RenderPass& pass = AddPass(RG_PASS_2D);
    auto desc = BuildDefaultTextureDesc(DEFAULT_SURFACE_FORMAT,
                                        AttachmentType::COLOR_2D);
    desc.bindFlags |= BIND_SHADER_RESOURCE;

    RGTextureId depth = CreateTexture({
        RG_RES_DEPTH_STENCIL,
        BuildDefaultTextureDesc(RT_FMT_GBUFFER_DEPTH, AttachmentType::DEPTH_STENCIL_2D),
    });

    RGTextureId color = ImportTexture({ in.color_attachment });

    pass.WriteColor(color, {}, LoadOp::Clear)
        .WriteDepth(depth, {}, LoadOp::Clear, 0.0f, LoadOp::Clear)
        .SetExecuteFunc(Pass2DDrawFunc);
}

#if 0
void RenderGraphBuilderExt::AddVoxelizationPass() {
    auto& manager = m_graphicsManager;
    if (manager.GetBackend() != Backend::OPENGL) {
        return;
    }

    const int voxel_size = DVAR_GET_INT(gfx_voxel_size);
    GpuTextureDesc desc = BuildDefaultTextureDesc(PixelFormat::R16G16B16A16_FLOAT,
                                                  AttachmentType::RW_TEXTURE,
                                                  voxel_size, voxel_size);
    desc.dimension = Dimension::TEXTURE_3D;
    desc.mipLevels = LogTwo(voxel_size);
    desc.depth = voxel_size;
    desc.miscFlags |= RESOURCE_MISC_GENERATE_MIPS;

    SamplerDesc sampler(MinFilter::LINEAR_MIPMAP_LINEAR, MagFilter::POINT, AddressMode::BORDER);

    auto& pass = AddPass(RG_PASS_VOXELIZATION);
    pass.create(RG_RES_VOXEL_LIGHTING, { desc, sampler })
        .create(RG_RES_VOXEL_NORMAL, { desc, sampler })
        .Read(ResourceAccess::SRV, RG_RES_SHADOW_MAP)
        //.Read(ResourceAccess::SRV, RG_RES_LTC1)
        //.Read(ResourceAccess::SRV, RG_RES_LTC2)
        .Read(ResourceAccess::UAV, RG_RES_VOXEL_LIGHTING)
        .Read(ResourceAccess::UAV, RG_RES_VOXEL_NORMAL)
        .SetExecuteFunc(VoxelizationPassFunc);
}
#endif

/// Bloom

#if 0
void RenderGraphBuilderExt::AddBloomPass() {
    // Setup pass
    const int width = m_config.frameWidth;
    const int height = m_config.frameHeight;

    auto& setup_pass = AddPass(RG_PASS_BLOOM_SETUP);
    SamplerDesc sampler = LinearClampSampler();

    // @TODO: use mips instead of generate this many resources
    for (int i = 0, w = width, h = height; i < BLOOM_MIP_CHAIN_MAX; ++i, w /= 2, h /= 2) {
        DEV_ASSERT(width > 1);
        DEV_ASSERT(height > 1);

        auto texture_desc = BuildDefaultTextureDesc(PixelFormat::R16G16B16A16_FLOAT,
                                                    AttachmentType::COLOR_2D,
                                                    w, h);

        auto res_name = std::format(RG_RES_BLOOM_PREFIX "{}x{}", w, h);
        setup_pass.create(res_name, { texture_desc, sampler });
    }

    auto bloom_res = std::format(RG_RES_BLOOM_PREFIX "{}x{}", width, height);

    AddDependency(RG_PASS_FORWARD, RG_PASS_BLOOM_SETUP);
    setup_pass
        .Read(ResourceAccess::SRV, RG_RES_LIGHTING)
        .Read(ResourceAccess::UAV, bloom_res)
        .SetExecuteFunc(BloomSetupFunc);

    // Down Sample
    for (int i = 0, w = width, h = height; i < BLOOM_MIP_CHAIN_MAX - 1; ++i, w /= 2, h /= 2) {
        auto pass_name = std::format(RG_PASS_BLOOM_DOWN_PREFIX "{}", i);
        auto input = std::format(RG_RES_BLOOM_PREFIX "{}x{}", w, h);
        auto inout = std::format(RG_RES_BLOOM_PREFIX "{}x{}", w / 2, h / 2);
        auto& pass = AddPass(pass_name);
        pass.Read(ResourceAccess::SRV, input)
            .Read(ResourceAccess::UAV, inout)
            .SetExecuteFunc(BloomDownSampleFunc);
        if (i == 0) {
            AddDependency(RG_PASS_BLOOM_SETUP, pass_name);
        } else {
            std::string prev_pass = std::format(RG_PASS_BLOOM_DOWN_PREFIX "{}", i - 1);
            AddDependency(prev_pass, pass_name);
        }
    }

    // Up Sample
    for (int i = 0, w = width, h = height; i < BLOOM_MIP_CHAIN_MAX - 1; ++i, w /= 2, h /= 2) {
        auto pass_name = std::format(RG_PASS_BLOOM_UP_PREFIX "{}", i);
        auto mip_low = std::format(RG_RES_BLOOM_PREFIX "{}x{}", w / 2, h / 2);
        auto mip = std::format(RG_RES_BLOOM_PREFIX "{}x{}", w, h);

        auto& pass = AddPass(pass_name);
        pass.Read(ResourceAccess::UAV, mip)
            .Read(ResourceAccess::SRV, mip_low)
            .SetExecuteFunc(BloomUpSampleFunc);

        if (i == BLOOM_MIP_CHAIN_MAX - 2) {
            auto down_sample_pass = std::format(RG_PASS_BLOOM_DOWN_PREFIX "{}", BLOOM_MIP_CHAIN_MAX - 2);
            AddDependency(down_sample_pass, pass_name);
        } else {
            auto prev_pass = std::format(RG_PASS_BLOOM_UP_PREFIX "{}", i + 1);
            AddDependency(prev_pass, pass_name);
        }
    }
}

#endif

}  // namespace cave::render
