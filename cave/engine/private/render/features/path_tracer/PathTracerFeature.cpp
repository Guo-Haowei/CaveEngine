#include "PathTracerFeature.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/core/math/Utils.h"
#include "engine/private/render/rhi/RenderDevice.h"
#include "engine/private/render/render_graph/RenderGraph.h"

// @TODO: refactor
#include "PathTracer.h"

namespace cave::render {

// @TODO: refactor
static struct {
    PathTracer pt;
} s_glob;

namespace {

void PathTracerComputeFunc(RenderPassExcutionContext& ctx) {
    auto& cmd = ctx.command;

    cmd.setPipelineState(PipelineStateName::PathTracer);
    const auto& input = ctx.pass.uavs[0];

    DEV_ASSERT(input);

    const uint32_t width = input->desc.width;
    const uint32_t height = input->desc.height;
    const uint32_t work_group_x = math::CeilingDivision(width, 16);
    const uint32_t work_group_y = math::CeilingDivision(height, 16);

    cmd.dispatch(work_group_x, work_group_y, 1);
}

void PathTracerPresentFunc(RenderPassExcutionContext& ctx) {
    CAVE_PROFILE_EVENT();

    auto& cmd = ctx.command;

    cmd.setPipelineState(PipelineStateName::Tone);
    cmd.setMesh(nullptr);
    cmd.draw(6);
}

}  // namespace

void PathTracerFeature::build(RenderGraph& graph,
                              const Inputs& inputs,
                              const RenderOptions&) {
    CAVE_PROFILE_EVENT();

    constexpr const char kPassPathTracerCompute[] = "p:pathtracer_compute";
    constexpr const char kPassPathTracerPresent[] = "p:pathtracer_present";
    constexpr const char kResPathTracer[] = "r:pathtracer";

    RGTextureId accumulated = graph.createResource({
        kResPathTracer,
        graph.buildDefaultTextureDesc(PixelFormat::R32G32B32A32_FLOAT,
                                      AttachmentType::COLOR_2D),
        LinearClampSampler(),
    });

    RGTextureId out = graph.importResource({ inputs.out });

    RenderPass& compute_pass = graph.addRenderPass(kPassPathTracerCompute);

    auto bvh_buffer = graph.importResource({ s_glob.pt.m_bvh_buffer });
    auto vertex_buffer = graph.importResource({ s_glob.pt.m_vertex_buffer });
    auto index_buffer = graph.importResource({ s_glob.pt.m_index_buffer });
    auto mesh_buffer = graph.importResource({ s_glob.pt.m_mesh_buffer });
    auto material_buffer = graph.importResource({ s_glob.pt.m_material_buffer });

    compute_pass
        .read(ResourceAccess::UAV, accumulated)
        .read(ResourceAccess::SRV, bvh_buffer)
        .read(ResourceAccess::SRV, vertex_buffer)
        .read(ResourceAccess::SRV, index_buffer)
        .read(ResourceAccess::SRV, mesh_buffer)
        .read(ResourceAccess::SRV, material_buffer)
        .read(ResourceAccess::SRV, inputs.skybox)
        .setExecuteFunc(PathTracerComputeFunc);

    RenderPass& present_pass = graph.addRenderPass(kPassPathTracerPresent);
    present_pass
        .read(ResourceAccess::SRV, accumulated)
        .writeColor(out, {}, LoadOp::Load)
        .setExecuteFunc(PathTracerPresentFunc);
}

bool PathTracerFeature::isActive() const {
    return s_glob.pt.isActive();
}

void PathTracerFeature::setMode(PathTracerMode mode) {
    s_glob.pt.setMode(mode);
}

void PathTracerFeature::update(Scene& scene) {
    s_glob.pt.update(scene);
}

}  // namespace cave::render
