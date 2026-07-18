#include "ShadowFeature.h"

#include "cave/core/diagnostics/Profiler.h"
#include "engine/private/render/render_graph/RenderGraph.h"

// @TODO: remove this
#include "engine/private/render/renderer/FrameData.h"
#include "engine/private/render/render_device/RenderDevice.h"

namespace cave::render {

// @TODO: maybe centralize the pass name
constexpr const char kPassShadow[] = "p:shadow";
constexpr const char kResShadowMap[] = "r:shadow";

extern void ExecuteDrawCommands(RenderPassExcutionContext& p_ctx,
                                const std::vector<DrawItem>& p_commands,
                                bool p_is_prepass);

static void ShadowPassFunc(RenderPassExcutionContext& p_ctx) {
    CAVE_PROFILE_EVENT();

    const auto& shadow_commands = p_ctx.frameData.commands[std::to_underlying(DrawPhase::Shadow)];
    if (shadow_commands.empty()) {
        return;
    }

    auto& cmd = p_ctx.cmd;
    const auto& frame = cmd.GetCurrentFrame();

    const PassContext& pass = p_ctx.frameData.shadowPasses[0];
    cmd.BindConstantBufferSlot<PerPassConstantBuffer>(frame.passCb.get(), pass.pass_idx);

    cmd.SetPipelineState(PSO_DPETH);
    ExecuteDrawCommands(p_ctx, shadow_commands, false);
}

ShadowFeature::Outputs ShadowFeature::Build(RenderGraph& p_graph, const RenderOptions&) {
    constexpr int shadow_res = 1024 * 2;
    DEV_ASSERT(math::IsPowerOfTwo(shadow_res));
    RenderPass& pass = p_graph.addRenderPass(kPassShadow);

    Outputs out{
        .shadow = p_graph.createTexture({
            kResShadowMap,
            p_graph.buildDefaultTextureDesc(PixelFormat::D32_FLOAT,
                                            AttachmentType::SHADOW_2D,
                                            shadow_res,
                                            shadow_res),
            ShadowMapSampler(),
        })
    };

    pass.writeDepth(out.shadow, {}, LoadOp::Clear)
        .setExecuteFunc(ShadowPassFunc);

    return out;
}

/// Shadow
#if 0
static void PointShadowPassFunc(RenderPassExcutionContext& p_ctx) {
    CRASH_NOW();

    auto& cmd = p_ctx.cmd;

    auto framebuffer = p_ctx.framebuffer;

    auto& frame = cmd.GetCurrentFrame();

    // prepare render data
    const auto [width, height] = framebuffer->GetBufferSize();

    const auto& shadow_commands = p_ctx.frameData.commands[std::to_underlying(DrawPhase::Shadow)];
    for (int pass_id = 0; pass_id < MAX_POINT_LIGHT_SHADOW_COUNT; ++pass_id) {
        auto& pass_ptr = p_ctx.frameData.pointShadowPasses[pass_id];
        if (!pass_ptr) {
            continue;
        }

        for (int face_id = 0; face_id < 6; ++face_id) {
            const uint32_t slot = pass_id * 6 + face_id;
            cmd.BindConstantBufferSlot<PointShadowConstantBuffer>(frame.pointShadowCb.get(), slot);

            cmd.SetRenderTarget(framebuffer, slot);
            cmd.Clear(framebuffer, CLEAR_DEPTH_BIT, nullptr, 1.0f, 0, slot);

            cmd.SetViewport(Viewport(width, height));

            cmd.SetPipelineState(PSO_POINT_SHADOW);
            ExecuteDrawCommands(p_ctx, shadow_commands, false);
        }
    }
}
#endif

}  // namespace cave::render
