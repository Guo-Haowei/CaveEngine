#include "BloomFeature.h"

#include "cave/core/diagnostics/Profiler.h"

#include "engine/private/render/render_graph/RenderGraph.h"
#include "engine/private/render/render_device/RenderDevice.h"

#define RG_PASS_BLOOM_DOWN_PREFIX "p:bloom_downsample"
#define RG_PASS_BLOOM_UP_PREFIX   "p:bloom_upsample"
#define RG_RES_BLOOM_PREFIX       "r:bloom"

namespace cave::render {

namespace {

constexpr int kBloomMipChainMax = 7;
constexpr const char kPassBloomSetup[] = "p:bloom_setup";

/// Bloom
void BloomSetupFunc(RenderPassExcutionContext& ctx) {
    CAVE_PROFILE_EVENT();

    auto& cmd = ctx.command;

    auto uav = ctx.pass.uavs[0];

    cmd.setPipelineState(PSO_BLOOM_SETUP);

    const uint32_t work_group_x = math::CeilingDivision(uav->desc.width, 16);
    const uint32_t work_group_y = math::CeilingDivision(uav->desc.height, 16);

    cmd.dispatch(work_group_x, work_group_y, 1);
}

void BloomDownSampleFunc(RenderPassExcutionContext& ctx) {
    CAVE_PROFILE_EVENT();

    auto& cmd = ctx.command;

    cmd.setPipelineState(PSO_BLOOM_DOWNSAMPLE);

    auto uav = ctx.pass.uavs[0];

    const uint32_t work_group_x = math::CeilingDivision(uav->desc.width, 16);
    const uint32_t work_group_y = math::CeilingDivision(uav->desc.height, 16);

    cmd.dispatch(work_group_x, work_group_y, 1);
}

void BloomUpSampleFunc(RenderPassExcutionContext& ctx) {
    CAVE_PROFILE_EVENT();

    auto& cmd = ctx.command;

    auto uav = ctx.pass.uavs[0];

    cmd.setPipelineState(PSO_BLOOM_UPSAMPLE);

    const uint32_t work_group_x = math::CeilingDivision(uav->desc.width, 16);
    const uint32_t work_group_y = math::CeilingDivision(uav->desc.height, 16);

    cmd.dispatch(work_group_x, work_group_y, 1);
}

}  // namespace

BloomFeature::Outputs BloomFeature::Build(RenderGraph& graph,
                                          const BloomFeature::Inputs& inputs,
                                          const RenderOptions&) {
    const auto& vp = graph.viewport();
    const int width = vp.w;
    const int height = vp.h;

    // @TODO: use mips instead of generate this many resources
    struct TextureContext {
        RGTextureId id;
        int w, h;
    };

    Vector<TextureContext> textures;
    for (int i = 0, w = width, h = height;
         i < kBloomMipChainMax && w > 1 && h > 1;
         ++i, w /= 2, h /= 2) {

        auto texture_desc = graph.buildDefaultTextureDesc(PixelFormat::R16G16B16A16_FLOAT,
                                                          AttachmentType::COLOR_2D,
                                                          w, h);

        auto res_name = std::format(RG_RES_BLOOM_PREFIX "@{}x{}", w, h);
        RGTextureId id = graph.createTexture({
            res_name,
            texture_desc,
            LinearClampSampler(),
        });
        textures.emplace_back(id, w, h);
    }

    const int num_textures = static_cast<int>(textures.size());
    DEV_ASSERT(num_textures > 1);

    // Setup pass
    RGDependencyId setup_dependency = graph.createDependency();

    RenderPass& setup_pass = graph.addRenderPass(kPassBloomSetup);
    setup_pass
        .readDependency(inputs.dependency)
        .read(ResourceAccess::SRV, inputs.color)
        .read(ResourceAccess::UAV, textures[0].id)
        .writeDependency(setup_dependency)
        .setExecuteFunc(BloomSetupFunc);

    RGDependencyId last_dep = setup_dependency;

    // Down Sample
    for (int i = 0; i < num_textures - 1; ++i) {
        const auto& dest = textures[i + 1];

        auto pass_name = std::format(RG_PASS_BLOOM_DOWN_PREFIX "@{}x{}", dest.w, dest.h);
        auto& pass = graph.addRenderPass(pass_name);
        RGDependencyId new_dep = graph.createDependency();
        pass.readDependency(last_dep)
            .read(ResourceAccess::SRV, textures[i].id)
            .read(ResourceAccess::UAV, dest.id)
            .writeDependency(new_dep)
            .setExecuteFunc(BloomDownSampleFunc);
        last_dep = new_dep;
    }

    RenderPass* last_pass = nullptr;
    // Up Sample
    for (int i = num_textures - 2; i >= 0; --i) {
        const auto& dest = textures[i];
        auto pass_name = std::format(RG_PASS_BLOOM_UP_PREFIX "@{}x{}", dest.w, dest.h);
        auto& pass = graph.addRenderPass(pass_name);
        last_pass = &pass;
        RGDependencyId new_dep = graph.createDependency();
        pass.readDependency(last_dep)
            .read(ResourceAccess::SRV, textures[i + 1].id)
            .read(ResourceAccess::UAV, dest.id)
            .writeDependency(new_dep)
            .setExecuteFunc(BloomUpSampleFunc);
        last_dep = new_dep;
    }

    auto upsample_dependency = graph.createDependency();
    last_pass->writeDependency(upsample_dependency);

    return {
        .dependency = upsample_dependency,
        .bloom = textures[0].id
    };
}

}  // namespace cave::render
