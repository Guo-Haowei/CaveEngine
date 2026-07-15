#include "PathTracerFeature.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/core/math/Utils.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/render/render_graph/RenderGraph.h"

// @TODO: refactor
#include "PathTracer.h"

namespace cave::render {

static constexpr const char RG_PASS_PATHTRACER_COMPUTE[] = "p:pathtracer_compute";
static constexpr const char RG_PASS_PATHTRACER_PRESENT[] = "p:pathtracer_present";
static constexpr const char RG_RES_PATHTRACER[] = "r:pathtracer";

static void PathTracerComputeFunc(RenderPassExcutionContext& p_ctx) {
    auto& cmd = p_ctx.cmd;

    cmd.SetPipelineState(PSO_PATH_TRACER);
    const auto& input = p_ctx.pass.uavs[0];

    DEV_ASSERT(input);

    const uint32_t width = input->desc.width;
    const uint32_t height = input->desc.height;
    const uint32_t work_group_x = math::CeilingDivision(width, 16);
    const uint32_t work_group_y = math::CeilingDivision(height, 16);

    // @TODO: transition
    BindPathTracerData(cmd);
    cmd.Dispatch(work_group_x, work_group_y, 1);
    UnbindPathTracerData(cmd);
}

static void PathTracerPresentFunc(RenderPassExcutionContext& p_ctx) {
    CAVE_PROFILE_EVENT();

    auto& cmd = p_ctx.cmd;

    // cmd.Clear(fb, CLEAR_COLOR_BIT);
    cmd.SetPipelineState(PSO_POST_PROCESS);
    cmd.SetMesh(nullptr);
    cmd.DrawArrays(6);
}

PathTracerFeature::Outputs PathTracerFeature::Build(RenderGraph& p_graph,
                                                    const RenderOptions&,
                                                    const Inputs& p_in) {
    CAVE_PROFILE_EVENT();

    RGTextureId accumulated = p_graph.createTexture({
        RG_RES_PATHTRACER,
        p_graph.buildDefaultTextureDesc(PixelFormat::R32G32B32A32_FLOAT,
                                        AttachmentType::COLOR_2D),
        LinearClampSampler(),
    });

    RGTextureId out = p_graph.importTexture({ p_in.out });

    RenderPass& compute_pass = p_graph.addRenderPass(RG_PASS_PATHTRACER_COMPUTE);

    compute_pass
        .read(ResourceAccess::UAV, accumulated)
        .setExecuteFunc(PathTracerComputeFunc);

    RenderPass& present_pass = p_graph.addRenderPass(RG_PASS_PATHTRACER_PRESENT);
    present_pass
        .read(ResourceAccess::SRV, accumulated)
        .writeColor(out, {}, LoadOp::Load)
        .setExecuteFunc(PathTracerPresentFunc);

    return {};
}

// @TODO: refactor
static struct {
    PathTracer pt;
} s_glob;

void RequestPathTracerUpdate(Scene& p_scene) {
    // @TODO: refactor
    s_glob.pt.Update(p_scene);
}

// path tracer
void SetPathTracerMode(PathTracerMode p_mode) {
    s_glob.pt.SetMode(p_mode);
}

bool IsPathTracerActive() {
    return s_glob.pt.IsActive();
}

void BindPathTracerData(IRenderDevice& p_device) {
    s_glob.pt.BindData(p_device);
}

void UnbindPathTracerData(IRenderDevice& p_device) {
    s_glob.pt.UnbindData(p_device);
}

}  // namespace cave::render
