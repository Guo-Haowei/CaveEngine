#include "CommonPasses.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/render/rhi/Sampler.h"
#include "cave/runtime/display/DisplayService.h"

#include "RenderGraphDefines.h"
#include "RenderPass.h"

// @TODO: remove
#include "engine/private/render/renderer/FrameData.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {
#include "shader_resource_defines.slang.h"
}  // namespace cave

namespace cave::render {

using math::Vec2f;

constexpr const char kPass2D[] = "p:2d";
constexpr const char kPassDepthPrepass[] = "p:depth_prepass";
constexpr const char kPassGbuffer[] = "p:gbuffer";
constexpr const char kPassLighting[] = "p:lighting";
constexpr const char kPassForward[] = "p:forward";
constexpr const char kPassOutline[] = "p:outline";
constexpr const char kPassPostProcess[] = "p:post_process";
constexpr const char kPassOverlay[] = "p:overlay";

constexpr const char kResDepthStencil[] = "r:depth";
constexpr const char kResColor0[] = "r:gbuffer0";
constexpr const char kResColor1[] = "r:gbuffer1";
constexpr const char kResColor2[] = "r:gbuffer2";
constexpr const char kResLighting[] = "r:lighting";
constexpr const char kResOutline[] = "r:outline";

extern void DepthPrepassFunc(RenderPassExcutionContext& ctx);
extern void GbufferPassFunc(RenderPassExcutionContext& ctx);
extern void HighlightPassFunc(RenderPassExcutionContext& ctx);
extern void LightingPassFunc(RenderPassExcutionContext& ctx);
extern void ForwardPassFunc(RenderPassExcutionContext& ctx);
extern void TonePassFunc(RenderPassExcutionContext& ctx);
extern void OverlayDrawFunc(RenderPassExcutionContext& ctx);

DepthPrepassOutput RenderGraphBuilderExt::addDepthPrepass() {
    RenderPass& pass = addRenderPass(kPassDepthPrepass);

    DepthPrepassOutput out{
        .depth = createResource({
            kResDepthStencil,
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
        .color0 = createResource({
            kResColor0,
            buildDefaultTextureDesc(RT_FMT_GBUFFER_BASE_COLOR, AttachmentType::COLOR_2D),
        }),
        .color1 = createResource({
            kResColor1,
            buildDefaultTextureDesc(RT_FMT_GBUFFER_NORMAL, AttachmentType::COLOR_2D),
        }),
        .color2 = createResource({
            kResColor2,
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
        .lighting = createResource({
            kResLighting,
            buildDefaultTextureDesc(kFormatLighting, AttachmentType::COLOR_2D),
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

    ForwardOutput out = {
        .dependency = createDependency(),
    };

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

    return out;
}

HighlightOutput RenderGraphBuilderExt::addHighlightPass(const HighlightInput& p_in) {
    RenderPass& pass = addRenderPass(kPassOutline);

    HighlightOutput out = {
        .outline = createResource({
            .debug_name = kResOutline,
            .resource_desc = buildDefaultTextureDesc(kFormatOutlineSelect, AttachmentType::COLOR_2D),
            .sampler_desc = PointClampSampler(),
        }),
    };

    pass.readDepth(p_in.stencil, {}, LoadOp::Load)
        .writeColor(out.outline, {}, LoadOp::Clear)
        .setExecuteFunc(HighlightPassFunc);

    return out;
}

PostProcessOutput RenderGraphBuilderExt::addPostProcessPass(const PostProcessInput& in) {
    RenderPass& pass = addRenderPass(kPassPostProcess);
    auto desc = buildDefaultTextureDesc(kFormatTone,
                                        AttachmentType::COLOR_2D);
    desc.bind_flags |= BIND_SHADER_RESOURCE;

    PostProcessOutput out{
        .dependency = createDependency(),
        .processed = importResource({ in.color_attachment }),
    };

    pass.readDependency(in.dependency)
        .read(ResourceAccess::SRV, in.lighting)
        .read(ResourceAccess::SRV, in.outline)
        .read(ResourceAccess::SRV, in.bloom);

    pass.writeDependency(out.dependency)
        .writeColor(out.processed, {}, LoadOp::Clear)
        .setExecuteFunc(TonePassFunc);

    return out;
}

Pass2DOutput RenderGraphBuilderExt::add2dPass(const Pass2DInput& in) {
    RenderPass& pass = addRenderPass(kPass2D);

    RGTextureId depth = createResource({
        kResDepthStencil,
        buildDefaultTextureDesc(RT_FMT_GBUFFER_DEPTH, AttachmentType::DEPTH_STENCIL_2D),
    });

    RGTextureId color = importResource({ in.color_attachment });
    RGDependencyId dependency = createDependency();

    pass.writeDependency(dependency)
        .writeColor(color, {}, LoadOp::Clear)
        .writeDepth(depth, {}, LoadOp::Clear, 0.0f, LoadOp::Clear);

    return Pass2DOutput{
        .dependency = dependency,
        .color_attachment = in.color_attachment,
    };
}

void RenderGraphBuilderExt::addOverlayPass(const OverlayInput& in) {
    RenderPass& pass = addRenderPass(kPassOverlay);

    RGTextureId color = importResource({ in.color_attachment });
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
    GpuResourceDesc desc = BuildDefaultTextureDesc(PixelFormat::R16G16B16A16_FLOAT,
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

}  // namespace cave::render
