#include "PathTracerFeature.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/core/math/Utils.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/render/render_graph/RenderGraph.h"

// @TODO: refactor
#include "PathTracer.h"

namespace cave::render {

namespace {

constexpr const char kPassPathTracerCompute[] = "p:pathtracer_compute";
constexpr const char kPassPathTracerPresent[] = "p:pathtracer_present";
constexpr const char kResPathTracer[] = "r:pathtracer";

void PathTracerComputeFunc(RenderPassExcutionContext& ctx) {
    auto& cmd = ctx.command;

    cmd.setPipelineState(PSO_PATH_TRACER);
    const auto& input = ctx.pass.uavs[0];

    DEV_ASSERT(input);

    const uint32_t width = input->desc.width;
    const uint32_t height = input->desc.height;
    const uint32_t work_group_x = math::CeilingDivision(width, 16);
    const uint32_t work_group_y = math::CeilingDivision(height, 16);

    // @TODO: transition
    BindPathTracerData(cmd);
    cmd.dispatch(work_group_x, work_group_y, 1);
    UnbindPathTracerData(cmd);
}

void PathTracerPresentFunc(RenderPassExcutionContext& ctx) {
    CAVE_PROFILE_EVENT();

    auto& cmd = ctx.command;

    cmd.setPipelineState(PSO_POST_PROCESS);
    cmd.setMesh(nullptr);
    cmd.draw(6);
}

}  // namespace

PathTracerFeature::Outputs PathTracerFeature::build(RenderGraph& graph,
                                                    const Inputs& inputs,
                                                    const RenderOptions&) {
    CAVE_PROFILE_EVENT();

    RGTextureId accumulated = graph.createTexture({
        kResPathTracer,
        graph.buildDefaultTextureDesc(PixelFormat::R32G32B32A32_FLOAT,
                                      AttachmentType::COLOR_2D),
        LinearClampSampler(),
    });

    RGTextureId out = graph.importTexture({ inputs.out });

    RenderPass& compute_pass = graph.addRenderPass(kPassPathTracerCompute);

    // @TODO: read GpuStructuredBuffer as UAV
    compute_pass
        .read(ResourceAccess::UAV, accumulated)
        .setExecuteFunc(PathTracerComputeFunc);

    RenderPass& present_pass = graph.addRenderPass(kPassPathTracerPresent);
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
    s_glob.pt.update(p_scene);
}

// path tracer
void SetPathTracerMode(PathTracerMode p_mode) {
    s_glob.pt.setMode(p_mode);
}

bool IsPathTracerActive() {
    return s_glob.pt.isActive();
}

void BindPathTracerData(ICommandList& cmd) {
    s_glob.pt.bindData(cmd);
}

void UnbindPathTracerData(ICommandList& cmd) {
    s_glob.pt.unbindData(cmd);
}

}  // namespace cave::render
