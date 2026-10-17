#include "Renderer.h"

#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/core/diagnostics/Profiler.h"
#include "cave/render/components/MeshRendererComponent.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/ui/IUIRuntime.h"

#include "FramePlan.h"
#include "RendererDebug.h"
#include "RenderScene.h"
#include "RenderSceneBuilder.h"
#include "RenderSubmission.h"
#include "TransientPool.h"

#include "engine/private/core/math/MatrixTransform.h"
#include "engine/private/render/features/PrecomputedTextures.h"
#include "engine/private/render/features/path_tracer/PathTracerFeature.h"
#include "engine/private/render/features/BloomFeature.h"
#include "engine/private/render/features/EnvironmentFeature.h"
#include "engine/private/render/features/ShadowFeature.h"
#include "engine/private/render/features/SsaoFeature.h"
#include "engine/private/render/render_graph/CompiledGraph.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

// @TODO: cleanup
#include "engine/private/render/render_graph/CommonPasses.h"
#include "engine/private/render/render_graph/RenderGraphDefines.h"

#include "engine/private/render/renderer/GraphicsDvars.h"

namespace cave::render {

using math::Vec2f;
using math::Vec2i;
using math::Vec3f;
using math::Vec4f;

class Renderer::Impl {
public:
    Impl(EngineServices& services);

    auto initialize() -> Result<void>;
    void finalize();

    void tick(const FrameTime& time, std::span<const ResolvedView> views);

    const RendererCapabilities& getCapabilities() const { return m_capabilities; }

    void setMode(bool is_2d) { m_is_2d = is_2d; }

#if USING(USE_RENDERER_DEBUG)
    PoolSnapshot poolSnapshot() const { return m_transient_pool.snapshot(); }
#endif

#if USING(USE_COMMAND)
    bool cmdDump(CommandContext& ctx, const CommandArgs& args);
#endif

    PathTracerFeature& pathtracer() {
        return m_pathtracer;
    }

private:
    FramePlan buildFramePlan(const FrameTime& frame,
                             std::span<const ResolvedView> views);

    auto buildRenderGraph(const RenderOptions& plan,
                          const ResolvedView& view) -> Result<Ref<CompiledGraph>>;

    auto buildRenderGraphDeferred(const RenderOptions& plan,
                                  const ResolvedView& view) -> Result<Ref<CompiledGraph>>;

    auto buildRenderGraphPt(const RenderOptions& plan,
                            const ResolvedView& view) -> Result<Ref<CompiledGraph>>;

    auto buildRenderGraph2d(const RenderOptions& plan,
                            const ResolvedView& view) -> Result<Ref<CompiledGraph>>;

    RenderScene& getOrCreateRenderScene(SceneId scene_id);

private:
    EngineServices& m_services;
    IRenderDevice& m_device;
    RenderSceneBuilder m_scene_builder;
    HashMap<SceneId, RenderScene> m_scene_cache;

    // features
    TransientPool m_transient_pool;
    BloomFeature m_bloom;
    EnvironmentFeature m_env;
    ShadowFeature m_shadow;
    SsaoFeature m_ssao;
    PathTracerFeature m_pathtracer;

    GpuTextureId m_brdf{};
    GpuTextureId m_ltc1{};
    GpuTextureId m_ltc2{};

    RendererCapabilities m_capabilities{};
    bool m_is_2d{ false };
};

Renderer::Renderer(EngineServices& services)
    : IService("Renderer")
    , m_impl(MakeOwner<Impl>(services))
    , m_overlay_renderer(MakeOwner<OverlayRenderer>(services.assetRegistry()))
    , m_ui_renderer(MakeOwner<UIRenderer>(services.assetRegistry())) {

    m_overlay_renderer->setScreenSpace(false);
    m_ui_renderer->setScreenSpace(true);
}

Renderer::~Renderer() = default;

auto Renderer::InitializeImpl() -> Result<void> {
    return m_impl->initialize();
}

void Renderer::FinalizeImpl() {
    m_impl->finalize();
}

void Renderer::tick(const FrameTime& time, std::span<const ResolvedView> resolved_views) {
    m_impl->tick(time, resolved_views);
}

const RendererCapabilities& Renderer::getCapabilities() const {
    return m_impl->getCapabilities();
}

void Renderer::setMode(bool is_2d) {
    m_impl->setMode(is_2d);
}

PathTracerFeature& Renderer::pathtracer() {
    return m_impl->pathtracer();
}

// @TODO: remove this
extern void RunMeshRenderSystem(const Scene& scene,
                                const RenderScene& rscene,
                                const ResolvedView& view,
                                FrameData& out_data);

#if 0
static void DebugDrawBVH(int p_level, BvhAccel* p_bvh, const Mat4f* p_matrix) {
    if (!p_bvh || p_bvh->depth > p_level) {
        return;
    }

    if (p_bvh->depth == p_level) {
        renderer::AddDebugCube(p_bvh->aabb,
                               Color::HexRgba(0xFFFF0037),
                               p_matrix);
    }

    DebugDrawBVH(p_level, p_bvh->left.get(), p_matrix);
    DebugDrawBVH(p_level, p_bvh->right.get(), p_matrix);
};
#endif

// @TODO: refactor
static void FillConstantBuffer(const FrameTime& p_frame,
                               const Scene* p_scene,
                               const ResolvedView& p_view,
                               FrameData& p_out_data) {
    const auto& options = p_out_data.options;
    auto& cache = p_out_data.perFrameCache;

    cache.c_screen_size.x = (float)p_view.viewport_px.w;
    cache.c_screen_size.y = (float)p_view.viewport_px.h;

    // camera
    {
        const CameraParams& cam = p_view.cam;
        cache.c_cam_view = cam.view;
        cache.c_cam_proj = cam.proj;
        cache.c_inv_cam_view = cam.view_inv;
        cache.c_inv_cam_proj = cam.proj_inv;
        cache.c_cam_fovy = p_view.fovy_rad;
        cache.c_cam_forward = (cam.view_inv * -Vec4f::UnitZ);
        cache.c_cam_right = (cam.view_inv * Vec4f::UnitX);
        cache.c_cam_up = (cam.view_inv * Vec4f::UnitY);
        cache.c_cam_pos = (cam.view_inv * Vec4f::UnitW);
    }

    // Bloom
    {
        cache.c_bloom_threshold = DVAR_GET_FLOAT(gfx_bloom_threshold);
        cache.c_enable_bloom = options.enable_bloom;

        cache.c_debug_voxel = options.debug_voxel_id;
        cache.c_pt_object_count = p_scene ? ((int)p_scene->count<MeshRendererComponent>()) : 0;
    }

    // IBL
    {
        cache.c_enable_ibl = options.enable_ibl;
    }

    // SSAO
    {
        // @TODO: do this properly
        static auto kernel_data = SsaoFeature::createKernel();
        cache.c_enable_ssao = options.enable_ssao;
        cache.c_ssao_kernal_radius = options.ssao_kernel_radius;
        constexpr size_t kernel_size = sizeof(kernel_data);
        static_assert(sizeof(cache.c_ssao_kernel) == kernel_size);
        memcpy(cache.c_ssao_kernel, kernel_data.data(), kernel_size);
    }

    cache.c_frame_index = static_cast<uint32_t>(p_frame.frame_index);
    // @TODO: fix this
    cache.c_reset_accumulation = true;
    cache.c_reset_accumulation = false;
}

static void FillPerPassConstants(const CompiledGraph& graph, FrameData& out_data) {
    int idx = 0;
    out_data.passCache.resize(graph.getCompiledPass().size());
    for (const CompiledPass& pass : graph.getCompiledPass()) {
        auto& cache = out_data.passCache[idx++];
        std::memset(&cache, 0, sizeof(cache));
        DEV_ASSERT(pass.srvs.size() <= std::size(cache.c_pass_textures));
        for (size_t slot = 0; slot < pass.srvs.size(); ++slot) {
            const auto& texture = pass.srvs[slot];
            if (texture) {
                cache.c_pass_textures[slot].x = (uint32_t)texture->getResidentHandle();
            }
        }

        uint* uavs = &cache.c_output_uav0;
        for (size_t slot = 0; slot < pass.uavs.size(); ++slot) {
            if (const auto& texture = pass.uavs[slot]) {
                uavs[slot] = (uint32_t)texture->getUavHandle();
            }
        }
    }
}

static void FillEnvConstants(FrameData& out_data) {
    constexpr int count = kIBLMipChainMax * 6;
    if (out_data.batchCache.buffer.size() < count) {
        out_data.batchCache.buffer.resize(count);
    }

    auto matrices = out_data.options.is_opengl ? math::BuildOpenGlCubeMapViewProjectionMatrix(Vec3f(0)) : BuildCubeMapViewProjectionMatrix(Vec3f(0));
    for (int mip_idx = 0; mip_idx < kIBLMipChainMax; ++mip_idx) {
        for (int face_id = 0; face_id < 6; ++face_id) {
            auto& batch = out_data.batchCache.buffer[mip_idx * 6 + face_id];
            batch.c_cube_proj_view_matrix = matrices[face_id];
            batch.c_env_pass_roughness = (float)mip_idx / (float)(kIBLMipChainMax - 1);
        }
    }
}

Renderer::Impl::Impl(EngineServices& services)
    : m_services(services)
    , m_device(services.renderDevice())
    , m_transient_pool(m_device)
    , m_env(m_transient_pool, m_device)
    , m_ssao(m_device)
    , m_pathtracer(m_device) {
}

auto Renderer::Impl::initialize() -> Result<void> {
    m_capabilities.canRunBloom = m_device.getCapabilities().support_cs;
    m_capabilities.canRunIBL = true;
    return Result<void>();
}

void Renderer::Impl::finalize() {
}

void Renderer::Impl::tick(const FrameTime& time, std::span<const ResolvedView> views) {
    CAVE_PROFILE_EVENT();

    auto submission = MakeOwner<RenderSubmission>();

    FramePlan plan = buildFramePlan(time, views);

    if (!views.empty()) {
        if (Scene* scene = views[0].scene) {
            m_pathtracer.update(*scene);
        }
    }

    for (size_t idx = 0; idx < plan.frame_data.size(); ++idx) {
        const ResolvedView& view = plan.views[idx];
        FrameData& data = plan.frame_data[idx];
        data.view_id = view.view_id;

        if (auto res = buildRenderGraph(data.options, view); !res) {
            CRASH_NOW();
        } else {
            auto graph = *res;
            graph->resolveTextures(m_transient_pool);

            submission->render_graph.push_back(graph);

            FillPerPassConstants(*graph, plan.frame_data[idx]);
        }
    }

    submission->frame_data = std::move(plan.frame_data);

    m_device.submit(std::move(submission));
}

FramePlan Renderer::Impl::buildFramePlan(const FrameTime& time,
                                         std::span<const ResolvedView> views) {
    FramePlan plan;

    const bool is_opengl = m_device.backend() == Backend::OpenGl;
    RenderOptions options = {
        .enable_ssao = DVAR_GET_BOOL(gfx_ssao_enabled),
        .enable_bloom = m_capabilities.canRunBloom && DVAR_GET_BOOL(gfx_enable_bloom),
        .enable_ibl = m_capabilities.canRunIBL && DVAR_GET_BOOL(gfx_enable_ibl),
        .enable_vxgi = false,
        .is_opengl = is_opengl,
        .debug_voxel_id = DVAR_GET_INT(gfx_debug_vxgi_voxel),
        .debug_bvh_depth = DVAR_GET_INT(gfx_bvh_debug),
        .voxel_texture_size = DVAR_GET_INT(gfx_voxel_size),
        .ssao_kernel_radius = DVAR_GET_FLOAT(gfx_ssao_radius),
    };

    plan.frame_data.resize(views.size());
    plan.views.reserve(views.size());

    int view_idx = 0;

    for (const ResolvedView& view : views) {
        RenderScene& render_scene = getOrCreateRenderScene(view.scene_id);
        m_scene_builder.BuildFull(*view.scene, render_scene);

        plan.views.emplace_back(view);

        FrameData& framedata = plan.frame_data[view_idx];
        framedata.options = options;

        FillConstantBuffer(time, view.scene, view, framedata);

        RunMeshRenderSystem(*view.scene, render_scene, view, framedata);

        FillEnvConstants(framedata);

        ++view_idx;
    }

    return plan;
}

auto Renderer::Impl::buildRenderGraph(const RenderOptions& plan,
                                      const ResolvedView& view) -> Result<Ref<CompiledGraph>> {
    if (m_pathtracer.isActive()) [[unlikely]] {
        return buildRenderGraphPt(plan, view);
    }

    return m_is_2d ? buildRenderGraph2d(plan, view) : buildRenderGraphDeferred(plan, view);
}

auto Renderer::Impl::buildRenderGraphDeferred(const RenderOptions& plan,
                                              const ResolvedView& view) -> Result<Ref<CompiledGraph>> {
    if (!m_brdf) {
        Ref<ImageAsset> image = m_services.assetManager().findImage("brdf.hdr");
        m_brdf = m_device.createTexture(image.get());
    }
    if (!m_ltc1) {
        m_ltc1 = CreateLTC1(m_device);
    }
    if (!m_ltc2) {
        m_ltc2 = CreateLTC2(m_device);
    }

    RenderGraphBuilderExt graph(view.viewport_px);

    RGTextureId brdf = graph.importResource({ m_brdf });
    RGTextureId ltc1 = graph.importResource({ m_ltc1 });
    RGTextureId ltc2 = graph.importResource({ m_ltc2 });

    auto env_outputs = m_env.build(graph, plan);

    auto shadow_outputs = m_shadow.build(graph, plan);

    // @TODO: refactor the following
    auto prepass_outputs = graph.addDepthPrepass();

    auto gbuffer_outputs = graph.addGbufferPass({
        .depth = prepass_outputs.depth,
    });

    SsaoFeature::Outputs ssao_outputs{};

    if (plan.enable_ssao) {
        SsaoFeature::Inputs ssao_inputs{
            .normal = gbuffer_outputs.color1,
            .depth = prepass_outputs.depth,
        };
        ssao_outputs = m_ssao.build(graph, ssao_inputs, plan);
    }

    auto lighting_outputs = graph.addLightingPass({
        .color0 = gbuffer_outputs.color0,
        .color1 = gbuffer_outputs.color1,
        .color2 = gbuffer_outputs.color2,
        .depth = prepass_outputs.depth,
        .ssao = ssao_outputs.processed,
        .shadow = shadow_outputs.shadow,
        .ibl_diffuse = env_outputs.ibl_diffuse,
        .ibl_prefiltered = env_outputs.ibl_prefiltered,
        .brdf = brdf,
        .ltc1 = ltc1,
        .ltc2 = ltc2,
    });

    auto forward_outputs = graph.addForwardPass({
        .dependency = lighting_outputs.dependency,
        .skybox = env_outputs.skybox,
        .shadow = shadow_outputs.shadow,
        .ibl_diffuse = env_outputs.ibl_diffuse,
        .ibl_prefiltered = env_outputs.ibl_prefiltered,
        .brdf = brdf,
        .ltc1 = ltc1,
        .ltc2 = ltc2,
        .depth = prepass_outputs.depth,
        .lighting = lighting_outputs.lighting,
    });

    BloomFeature::Outputs bloom_output = {
        .dependency = RGDependencyId::null(),
        .bloom = RGTextureId::null(),
    };

    if (plan.enable_bloom) {
        bloom_output = m_bloom.build(graph,
                                     {
                                         .dependency = forward_outputs.dependency,
                                         .color = lighting_outputs.lighting,
                                     },
                                     plan);
    }

    auto highlight_output = graph.addHighlightPass({
        .stencil = prepass_outputs.depth,
    });

    auto post_process_output = graph.addPostProcessPass({
        .dependency = bloom_output.dependency,
        .lighting = lighting_outputs.lighting,
        .outline = highlight_output.outline,
        .bloom = bloom_output.bloom,
        .color_attachment = view.output,
    });

    graph.addOverlayPass({
        .dependency = post_process_output.dependency,
        .color_attachment = view.output,
    });

    return graph.compile();
}

auto Renderer::Impl::buildRenderGraph2d(const RenderOptions&,
                                        const ResolvedView& view) -> Result<Ref<CompiledGraph>> {

    RenderGraphBuilderExt graph(view.viewport_px);

    auto output = graph.add2dPass({ .color_attachment = view.output });
    graph.addOverlayPass(output);

    return graph.compile();
}

auto Renderer::Impl::buildRenderGraphPt(const RenderOptions& plan,
                                        const ResolvedView& view) -> Result<Ref<CompiledGraph>> {
    RenderGraph graph(view.viewport_px);

    auto env_outputs = m_env.build(graph, plan);

    PathTracerFeature::Inputs inputs{
        .skybox = env_outputs.skybox,
        .out = view.output,
    };

    m_pathtracer.build(graph, inputs, plan);

    return graph.compile();
}

RenderScene& Renderer::Impl::getOrCreateRenderScene(SceneId scene_id) {
    return m_scene_cache[scene_id];
}

#if USING(USE_RENDERER_DEBUG)
PoolSnapshot Renderer::poolSnapshot() const {
    return m_impl->poolSnapshot();
}
#endif

#if USING(USE_COMMAND)
bool Renderer::cmdDump(CommandContext& ctx, const CommandArgs& args) {
    return m_impl->cmdDump(ctx, args);
}

bool Renderer::Impl::cmdDump(CommandContext& ctx, const CommandArgs& args) {
    RenderPoolDump_Cmd(m_transient_pool, ctx, args);
    return true;
}
#endif

}  // namespace cave::render
