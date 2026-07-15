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

constexpr const char kPass2D[] = "p:2d";
constexpr const char kPassDepthPrepass[] = "p:depth_prepass";
constexpr const char kPassGbuffer[] = "p:gbuffer";
constexpr const char kPassLighting[] = "p:lighting";
constexpr const char kPassForward[] = "p:forward";
constexpr const char kPassBloomSetup[] = "p:bloom_setup";
constexpr const char kPassOutline[] = "p:outline";
constexpr const char kPassPostProcess[] = "p:post_process";
constexpr const char kPassOverlay[] = "p:overlay";

constexpr const char RG_RES_DEPTH_STENCIL[] = "r:depth";
constexpr const char RG_RES_GBUFFER_COLOR0[] = "r:gbuffer0";
constexpr const char RG_RES_GBUFFER_COLOR1[] = "r:gbuffer1";
constexpr const char RG_RES_GBUFFER_COLOR2[] = "r:gbuffer2";
constexpr const char RG_RES_LIGHTING[] = "r:lighting";
constexpr const char RG_RES_POST_PROCESS[] = "r:post_process";
constexpr const char RG_RES_VOXEL_LIGHTING[] = "r:voxel_lighting";
constexpr const char RG_RES_VOXEL_NORMAL[] = "r:voxel_normal";
constexpr const char RG_RES_OUTLINE[] = "r:outline";

extern void DepthPrepassFunc(RenderPassExcutionContext& ctx);
extern void GbufferPassFunc(RenderPassExcutionContext& ctx);
extern void HighlightPassFunc(RenderPassExcutionContext& ctx);
extern void LightingPassFunc(RenderPassExcutionContext& ctx);
extern void ForwardPassFunc(RenderPassExcutionContext& ctx);
extern void BloomSetupFunc(RenderPassExcutionContext& ctx);
extern void BloomDownSampleFunc(RenderPassExcutionContext& ctx);
extern void BloomUpSampleFunc(RenderPassExcutionContext& ctx);
extern void TonePassFunc(RenderPassExcutionContext& ctx);
extern void Pass2DDrawFunc(RenderPassExcutionContext& ctx);
extern void OverlayDrawFunc(RenderPassExcutionContext& ctx);

DepthPrepassOutput RenderGraphBuilderExt::addDepthPrepass() {
    RenderPass& pass = addRenderPass(kPassDepthPrepass);

    DepthPrepassOutput out{
        .depth = createTexture({
            RG_RES_DEPTH_STENCIL,
            buildDefaultTextureDesc(RT_FMT_GBUFFER_DEPTH, AttachmentType::DEPTH_STENCIL_2D),
        }),
    };

    pass.writeDepth(out.depth, {}, LoadOp::Clear, 0.0f, LoadOp::Clear, STENCIL_FLAG_SKY)
        .setExecuteFunc(DepthPrepassFunc);

    return out;
}

GbufferOutput RenderGraphBuilderExt::addGbufferPass(const DepthPrepassOutput& p_in) {
    RenderPass& pass = addRenderPass(kPassGbuffer);

    GbufferOutput out{
        .color0 = createTexture({
            RG_RES_GBUFFER_COLOR0,
            buildDefaultTextureDesc(RT_FMT_GBUFFER_BASE_COLOR, AttachmentType::COLOR_2D),
        }),
        .color1 = createTexture({
            RG_RES_GBUFFER_COLOR1,
            buildDefaultTextureDesc(RT_FMT_GBUFFER_NORMAL, AttachmentType::COLOR_2D),
        }),
        .color2 = createTexture({
            RG_RES_GBUFFER_COLOR2,
            buildDefaultTextureDesc(RT_FMT_GBUFFER_MATERIAL, AttachmentType::COLOR_2D),
        }),
    };

    // @TODO: introduce versioning
    pass.readDepth(p_in.depth, {}, LoadOp::Load)
        .writeColor(out.color0, {}, LoadOp::Clear)
        .writeColor(out.color1, {}, LoadOp::Clear)
        .writeColor(out.color2, {}, LoadOp::Clear)
        .setExecuteFunc(GbufferPassFunc);
    return out;
}

LightingOutput RenderGraphBuilderExt::addLightingPass(const LightingInput& p_in) {
    LightingOutput out = {
        .dependency = createDependency(),
        .lighting = createTexture({
            RG_RES_LIGHTING,
            buildDefaultTextureDesc(RT_FMT_LIGHTING, AttachmentType::COLOR_2D),
        }),
    };


    RenderPass& pass = addRenderPass(kPassLighting);

    pass.read(ResourceAccess::SRV, p_in.color0)
        .read(ResourceAccess::SRV, p_in.color1)
        .read(ResourceAccess::SRV, p_in.color2)
        .read(ResourceAccess::SRV, p_in.depth)
        .read(ResourceAccess::SRV, p_in.ssao)
        .read(ResourceAccess::SRV, p_in.shadow)
        .read(ResourceAccess::SRV, p_in.ibl_diffuse)
        .read(ResourceAccess::SRV, p_in.ibl_prefiltered)
        .read(ResourceAccess::SRV, p_in.brdf)
        .read(ResourceAccess::SRV, p_in.ltc1)
        .read(ResourceAccess::SRV, p_in.ltc2);

    pass.writeDependency(out.dependency)
        .writeColor(out.lighting, {}, LoadOp::Clear)
        .setExecuteFunc(LightingPassFunc);

    return out;
}

ForwardOutput RenderGraphBuilderExt::addForwardPass(const ForwardInput& in) {
    RenderPass& pass = addRenderPass(kPassForward);

    pass.readDependency(in.dependency)
        .read(ResourceAccess::SRV, in.skybox)
        .read(ResourceAccess::SRV, in.shadow)
        .read(ResourceAccess::SRV, in.ibl_diffuse)
        .read(ResourceAccess::SRV, in.ibl_prefiltered)
        .read(ResourceAccess::SRV, in.brdf)
        .read(ResourceAccess::SRV, in.ltc1)
        .read(ResourceAccess::SRV, in.ltc2)
        .readDepth(in.depth, {}, LoadOp::Load);

    pass.writeColor(in.lighting, {}, LoadOp::Load)
        .setExecuteFunc(ForwardPassFunc);

    return ForwardOutput{};
}

HighlightOutput RenderGraphBuilderExt::addHighlightPass(const HighlightInput& p_in) {
    RenderPass& pass = addRenderPass(kPassOutline);

    HighlightOutput out = {
        .outline = createTexture({
            .debug_name = RG_RES_OUTLINE,
            .resourceDesc = buildDefaultTextureDesc(RT_FMT_OUTLINE_SELECT, AttachmentType::COLOR_2D),
            .samplerDesc = PointClampSampler(),
        }),
    };

    pass.readDepth(p_in.stencil, {}, LoadOp::Load)
        .writeColor(out.outline, {}, LoadOp::Clear)
        .setExecuteFunc(HighlightPassFunc);

    return out;
}

PostProcessOutput RenderGraphBuilderExt::addPostProcessPass(const PostProcessInput& in) {
    RenderPass& pass = addRenderPass(kPassPostProcess);
    auto desc = buildDefaultTextureDesc(RT_FMT_TONE,
                                        AttachmentType::COLOR_2D);
    desc.bindFlags |= BIND_SHADER_RESOURCE;

    PostProcessOutput out{
        .dependency = createDependency(),
        .processed = importTexture({ in.color_attachment }),
    };

    pass.read(ResourceAccess::SRV, in.lighting)
        .read(ResourceAccess::SRV, in.outline)
        .read(ResourceAccess::SRV, in.bloom);

    pass.writeDependency(out.dependency)
        .writeColor(out.processed, {}, LoadOp::Clear)
        .setExecuteFunc(TonePassFunc);

    return out;
}

Pass2DOutput RenderGraphBuilderExt::add2dPass(const Pass2DInput& in) {
    RenderPass& pass = addRenderPass(kPass2D);

    RGTextureId depth = createTexture({
        RG_RES_DEPTH_STENCIL,
        buildDefaultTextureDesc(RT_FMT_GBUFFER_DEPTH, AttachmentType::DEPTH_STENCIL_2D),
    });

    RGTextureId color = importTexture({ in.color_attachment });
    RGDependencyId dependency = createDependency();

    pass.writeDependency(dependency)
        .writeColor(color, {}, LoadOp::Clear)
        .writeDepth(depth, {}, LoadOp::Clear, 0.0f, LoadOp::Clear)
        .setExecuteFunc(Pass2DDrawFunc);

    return Pass2DOutput{
        .dependency = dependency,
        .color_attachment = in.color_attachment,
    };
}

void RenderGraphBuilderExt::addOverlayPass(const OverlayInput& in) {
    RenderPass& pass = addRenderPass(kPassOverlay);

    RGTextureId color = importTexture({ in.color_attachment });
    pass.writeColor(color, {}, LoadOp::Load)
        .readDependency(in.dependency)
        .setExecuteFunc(OverlayDrawFunc);
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
