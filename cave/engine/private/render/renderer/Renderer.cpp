#include "Renderer.h"

#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/ui/IUIRuntime.h"

#include "FramePlan.h"
#include "RendererDebug.h"
#include "RenderScene.h"
#include "RenderSceneBuilder.h"
#include "RenderSubmission.h"
#include "TransientPool.h"
#include "UIRenderer.h"

#include "engine/private/core/math/MatrixTransform.h"
#include "engine/private/render/features/PrecomputedTextures.h"
#include "engine/private/render/features/path_tracer/PathTracerFeature.h"
#include "engine/private/render/features/pbr/EnvironmentFeature.h"
#include "engine/private/render/features/shadow/ShadowFeature.h"
#include "engine/private/render/features/ssao/SsaoFeature.h"
#include "engine/private/render/render_graph/CompiledGraph.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

// @TODO: cleanup
#include "engine/private/render/render_graph/CommonPasses.h"
#include "engine/private/render/render_graph/RenderGraphDefines.h"

#include "engine/private/renderer/graphics_dvars.h"

namespace cave {

extern void RunTileMapRenderSystem(Scene* scene, FrameData& framedata);

extern void RunSpriteRenderSystem(const Scene* scene, FrameData& framedata);

}  // namespace cave

namespace cave::render {

using math::Vec2f;
using math::Vec2i;
using math::Vec3f;
using math::Vec4f;

class Renderer::Impl {
public:
    Impl(EngineServices& services)
        : m_services(services)
        , m_device(services.renderDevice())
        , m_transient_pool(m_device)
        , m_env(m_transient_pool, m_device)
        , m_ssao(m_device) {}

    void tick(const FrameTime& time, std::span<const ResolvedView> views);

    void setMode(bool is_2d) { m_is_2d = is_2d; }

#if USING(USE_COMMAND)
    bool Cmd_dump(CommandContext& ctx, const CommandArgs& args);
#endif

private:
    FramePlan buildFramePlan(const FrameTime& frame,
                             std::span<const ResolvedView> views);

    auto buildRenderGraph(const RenderOptions& plan,
                          const ResolvedView& view) -> Result<std::shared_ptr<CompiledGraph>>;

    auto buildRenderGraphDeferred(const RenderOptions& plan,
                                  const ResolvedView& view) -> Result<std::shared_ptr<CompiledGraph>>;

    auto buildRenderGraphPt(const RenderOptions& plan,
                            const ResolvedView& view) -> Result<std::shared_ptr<CompiledGraph>>;

    auto buildRenderGraph2d(const RenderOptions& plan,
                            const ResolvedView& view) -> Result<std::shared_ptr<CompiledGraph>>;

    RenderScene& getOrCreateRenderScene(SceneId scene_id);

private:
    EngineServices& m_services;
    IRenderDevice& m_device;
    RenderSceneBuilder scene_builder_;
    HashMap<SceneId, RenderScene> scene_cache_;

    // features
    TransientPool m_transient_pool;
    EnvironmentFeature m_env;
    ShadowFeature shadow_;
    SsaoFeature m_ssao;
    PathTracerFeature pathtracer_;

    GpuTextureId m_brdf{};
    GpuTextureId m_ltc1{};
    GpuTextureId m_ltc2{};
    bool m_is_2d{ false };
};

Renderer::Renderer(EngineServices& services)
    : m_impl(MakeOwner<Impl>(services))
    , m_overlay_renderer(MakeOwner<OverlayRenderer>(services.assetRegistry()))
    , m_ui_renderer(MakeOwner<UIRenderer>(services.assetRegistry())) {

    m_overlay_renderer->setScreenSpace(false);
    m_ui_renderer->setScreenSpace(false);
}

Renderer::~Renderer() = default;

void Renderer::tick(const FrameTime& time, std::span<const ResolvedView> resolved_views) {
    m_impl->tick(time, resolved_views);
}

void Renderer::setMode(bool is_2d) {
    m_impl->setMode(is_2d);
}

// @TODO: remove this
extern void runMeshRenderSystem(const Scene& scene,
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
static void fillConstantBuffer(const FrameTime& p_frame,
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
        cache.c_camView = cam.view;
        cache.c_camProj = cam.proj;
        cache.c_invCamView = cam.view_inv;
        cache.c_invCamProj = cam.proj_inv;
        cache.c_camera_fovy = p_view.fovy_rad;
        cache.c_cameraForward = (cam.view_inv * -Vec4f::UnitZ).xyz;
        cache.c_cameraRight = (cam.view_inv * Vec4f::UnitX).xyz;
        cache.c_cameraUp = (cam.view_inv * Vec4f::UnitY).xyz;
        cache.c_cameraPosition = (cam.view_inv * Vec4f::UnitW).xyz;
    }

    // Bloom
    {
        cache.c_bloomThreshold = 1.3f;
        cache.c_enableBloom = options.enable_bloom;

        cache.c_debugVoxelId = options.debugVoxelId;
        cache.c_ptObjectCount = p_scene ? ((int)p_scene->count<MeshRendererComponent>()) : 0;
    }

    // IBL
    {
        cache.c_iblEnabled = options.enable_ibl;
    }

    // SSAO
    {
        // @TODO: do this properly
        static auto kernel_data = SsaoFeature::CreateKernel();
        cache.c_ssaoEnabled = options.enable_ssao;
        cache.c_ssaoKernalRadius = options.ssaoKernelRadius;
        constexpr size_t kernel_size = sizeof(kernel_data);
        static_assert(sizeof(cache.c_ssaoKernel) == kernel_size);
        memcpy(cache.c_ssaoKernel, kernel_data.data(), kernel_size);
    }

    cache.c_frame_index = static_cast<uint32_t>(p_frame.frame_index);
    cache.c_scene_dirty = true;
}

static void fillEnvConstants(FrameData& out_data) {
    constexpr int count = IBL_MIP_CHAIN_MAX * 6;
    if (out_data.batchCache.buffer.size() < count) {
        out_data.batchCache.buffer.resize(count);
    }

    auto matrices = out_data.options.is_opengl ? math::BuildOpenGlCubeMapViewProjectionMatrix(Vec3f(0)) : BuildCubeMapViewProjectionMatrix(Vec3f(0));
    for (int mip_idx = 0; mip_idx < IBL_MIP_CHAIN_MAX; ++mip_idx) {
        for (int face_id = 0; face_id < 6; ++face_id) {
            auto& batch = out_data.batchCache.buffer[mip_idx * 6 + face_id];
            batch.c_cubeProjectionViewMatrix = matrices[face_id];
            batch.c_envPassRoughness = (float)mip_idx / (float)(IBL_MIP_CHAIN_MAX - 1);
        }
    }
}

template<typename T>
static GpuBufferDesc fillDesc(const std::vector<T>& data) {
    GpuBufferDesc desc{};
    desc.type = GpuBufferType::Vertex;
    desc.dynamic = true;
    desc.element_size = sizeof(T);
    desc.element_count = static_cast<uint32_t>(data.size());
    desc.initial_data = data.data();
    return desc;
}

// @TODO: move to UIRenderer
template<typename T>
static bool updateUIBuffer(IRenderDevice& device,
                           const std::vector<T>& data,
                           GpuBuffer* gpu_buffer) {
    if (data.size() > gpu_buffer->desc.element_count) {
        return false;
    }

    GpuBufferDesc desc = gpu_buffer->desc;
    desc.element_count = (uint32_t)data.size();
    desc.initial_data = data.data();
    device.UpdateBuffer(desc, gpu_buffer);
    return true;
}

void Renderer::Impl::tick(const FrameTime& time, std::span<const ResolvedView> views) {
    CAVE_PROFILE_EVENT();

    auto submission = MakeOwner<RenderSubmission>();

    FramePlan plan = buildFramePlan(time, views);

    for (size_t idx = 0; idx < plan.frame_data.size(); ++idx) {
        const ResolvedView& view = plan.views[idx];
        FrameData& data = plan.frame_data[idx];
        data.view_id = view.view_id;

        if (auto res = buildRenderGraph(data.options, view); !res) {
            CRASH_NOW();
        } else {
            auto graph = *res;
            graph->Resolve(m_transient_pool);

            submission->render_graph.push_back(graph);
        }
    }

    submission->frame_data = std::move(plan.frame_data);

    m_device.submit(std::move(submission));
}

FramePlan Renderer::Impl::buildFramePlan(const FrameTime& time,
                                         std::span<const ResolvedView> views) {
    FramePlan plan;

    const bool is_opengl = m_device.backend() == rhi::Backend::OpenGL;
    RenderOptions options = {
        .is_opengl = is_opengl,
        .enable_ssao = DVAR_GET_BOOL(gfx_ssao_enabled),
        .enable_bloom = DVAR_GET_BOOL(gfx_enable_bloom),
        .enable_ibl = DVAR_GET_BOOL(gfx_enable_ibl),

        .vxgiEnabled = false,
        .debugVoxelId = DVAR_GET_INT(gfx_debug_vxgi_voxel),
        .debugBvhDepth = DVAR_GET_INT(gfx_bvh_debug),
        .voxelTextureSize = DVAR_GET_INT(gfx_voxel_size),
        .ssaoKernelRadius = DVAR_GET_FLOAT(gfx_ssao_radius),
    };

    plan.frame_data.resize(views.size());
    plan.views.reserve(views.size());

    int view_idx = 0;

    for (const ResolvedView& view : views) {
        RenderScene& render_scene = getOrCreateRenderScene(view.scene_id);
        scene_builder_.BuildFull(*view.scene, render_scene);

        plan.views.push_back(view);

        FrameData& framedata = plan.frame_data[view_idx];
        framedata.options = options;

        fillConstantBuffer(time, view.scene, view, framedata);

        runMeshRenderSystem(*view.scene, render_scene, view, framedata);
        RunTileMapRenderSystem(view.scene, framedata);
        RunSpriteRenderSystem(view.scene, framedata);
        fillEnvConstants(framedata);

        // @HACK: only support first scene
        if (view_idx == 0) {
            RequestPathTracerUpdate(*view.scene);
        }

        ++view_idx;
    }

    return plan;
}

auto Renderer::Impl::buildRenderGraph(const RenderOptions& plan,
                                      const ResolvedView& view) -> Result<std::shared_ptr<CompiledGraph>> {
    if (IsPathTracerActive()) [[unlikely]] {
        return buildRenderGraphPt(plan, view);
    }

    return m_is_2d ? buildRenderGraph2d(plan, view) : buildRenderGraphDeferred(plan, view);
}

auto Renderer::Impl::buildRenderGraphDeferred(const RenderOptions& plan,
                                              const ResolvedView& view) -> Result<std::shared_ptr<CompiledGraph>> {
    if (!m_brdf) {
        Ref<ImageAsset> image = m_services.assetManager().findImage("brdf.hdr");
        m_brdf = m_device.CreateTexture(image.get());
    }
    if (!m_ltc1) {
        m_ltc1 = CreateLTC1(m_device);
    }
    if (!m_ltc2) {
        m_ltc2 = CreateLTC2(m_device);
    }

    RenderGraphBuilderExt graph(view.viewport_px);

    RGTextureId brdf = graph.ImportTexture({ m_brdf });
    RGTextureId ltc1 = graph.ImportTexture({ m_ltc1 });
    RGTextureId ltc2 = graph.ImportTexture({ m_ltc2 });

    auto env_outputs = m_env.Build(graph, plan);

    auto shadow_outputs = shadow_.Build(graph, plan);

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
        ssao_outputs = m_ssao.Build(graph, plan, ssao_inputs);
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

    auto highlight_outputs = graph.addHighlightPass({
        .stencil = prepass_outputs.depth,
    });

    graph.addPostProcessPass({
        .lighting = lighting_outputs.lighting,
        .outline = highlight_outputs.outline,
        .bloom = 0,
        .color_attachment = view.output,
    });

    return graph.Compile();
}

auto Renderer::Impl::buildRenderGraph2d(const RenderOptions& plan,
                                        const ResolvedView& view) -> Result<std::shared_ptr<CompiledGraph>> {
    unused(plan);

    RenderGraphBuilderExt graph(view.viewport_px);

    graph.add2dPass({ .color_attachment = view.output });

    return graph.Compile();
}

auto Renderer::Impl::buildRenderGraphPt(const RenderOptions& plan,
                                        const ResolvedView& view) -> Result<std::shared_ptr<CompiledGraph>> {
    RenderGraph graph(view.viewport_px);

    pathtracer_.Build(graph, plan, { view.output });

    return graph.Compile();
}

RenderScene& Renderer::Impl::getOrCreateRenderScene(SceneId scene_id) {
    return scene_cache_[scene_id];
}

#if USING(USE_COMMAND)
bool Renderer::Cmd_dump(CommandContext& ctx, const CommandArgs& args) {
    return m_impl->Cmd_dump(ctx, args);
}

bool Renderer::Impl::Cmd_dump(CommandContext& ctx, const CommandArgs& args) {
    RenderPoolDump_Cmd(m_transient_pool, ctx, args);
    return true;
}
#endif

}  // namespace cave::render
