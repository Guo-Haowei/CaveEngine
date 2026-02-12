#include "Renderer.h"

#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/math/MatrixTransform.h"
#include "engine/private/render/features/PrecomputedTextures.h"
#include "engine/private/render/features/path_tracer/PathTracerFeature.h"
#include "engine/private/render/features/pbr/EnvironmentFeature.h"
#include "engine/private/render/features/shadow/ShadowFeature.h"
#include "engine/private/render/features/ssao/SsaoFeature.h"
#include "engine/private/render/renderer/FramePlan.h"
#include "engine/private/render/renderer/RendererDebug.h"
#include "engine/private/render/renderer/RenderScene.h"
#include "engine/private/render/renderer/RenderSceneBuilder.h"
#include "engine/private/render/renderer/RenderSubmission.h"
#include "engine/private/render/renderer/TransientPool.h"
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

extern void RunTileMapRenderSystem(Scene* p_scene, FrameData& p_framedata);

extern void RunSpriteRenderSystem(const Scene* p_scene, FrameData& p_framedata);
extern void RunDebugRenderSystem(const Scene* p_scene, FrameData& p_framedata);

}  // namespace cave

namespace cave::render {

using math::Vector2i;
using math::Vector3f;
using math::Vector4f;

class Renderer::Impl {
public:
    Impl(IApplication& p_app)
        : m_app(p_app)
        , m_pool(*p_app.GetRenderDevice())
        , m_env(m_pool, *p_app.GetRenderDevice())
        , m_ssao(*p_app.GetRenderDevice()) {}

    auto Initialize() -> Result<void>;

    void Tick(const FrameTime& p_frame, std::span<const ResolvedView> p_views);

private:
    FramePlan BuildFramePlan(const FrameTime& p_frame, std::span<const ResolvedView> p_views);
    auto BuildRenderGraph(const RenderOptions& p_plan,
                          const ResolvedView& p_view) -> Result<std::shared_ptr<CompiledGraph>>;

    auto BuildRenderGraphDeferred(const RenderOptions& p_plan,
                                  const ResolvedView& p_view) -> Result<std::shared_ptr<CompiledGraph>>;

    auto BuildRenderGraphPathTracer(const RenderOptions& p_plan,
                                    const ResolvedView& p_view) -> Result<std::shared_ptr<CompiledGraph>>;

    RenderScene& GetOrCreateRenderScene(SceneId p_scene_id);

private:
    IApplication& m_app;
    RenderSceneBuilder m_scene_builder;
    std::unordered_map<SceneId, RenderScene> m_scene_cache;

    // features
    TransientPool m_pool;
    EnvironmentFeature m_env;
    ShadowFeature m_shadow;
    SsaoFeature m_ssao;
    PathTracerFeature m_pt;

    GpuTextureId m_brdf{};
    GpuTextureId m_ltc1{};
    GpuTextureId m_ltc2{};
};

Renderer::Renderer()
    : Module("Render") {}

Renderer::~Renderer() = default;

auto Renderer::InitializeImpl() -> Result<void> {
    m_impl = std::make_unique<Impl>(*m_app);
    return m_impl->Initialize();
}

void Renderer::FinalizeImpl() {
    m_impl.reset();
}

void Renderer::Tick(const FrameTime& p_frame, std::span<const ResolvedView> p_views) {
    m_impl->Tick(p_frame, p_views);
}

// @TODO: remove this
extern void RunMeshRenderSystem(const Scene& p_scene,
                                const RenderScene& p_rscene,
                                const ResolvedView& p_view,
                                FrameData& p_framedata);

#if 0
static void DebugDrawBVH(int p_level, BvhAccel* p_bvh, const Matrix4x4f* p_matrix) {
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

    // camera
    {
        const CameraParams& cam = p_view.cam;
        cache.c_camView = cam.view;
        cache.c_camProj = cam.proj;
        cache.c_invCamView = cam.view_inv;
        cache.c_invCamProj = cam.proj_inv;
        cache.c_cameraFovDegree = p_view.fovy;
        cache.c_cameraForward = cam.front;
        cache.c_cameraRight = cam.right;
        cache.c_cameraUp = cam.up;
        cache.c_cameraPosition = cam.position;
    }

    // Bloom
    {
        cache.c_bloomThreshold = 1.3f;
        cache.c_enableBloom = options.enable_bloom;

        cache.c_debugVoxelId = options.debugVoxelId;
        cache.c_ptObjectCount = p_scene ? ((int)p_scene->GetCount<MeshRendererComponent>()) : 0;
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

static void FillEnvConstants(FrameData& p_out_data) {
    // @TODO: return if necessary

    constexpr int count = IBL_MIP_CHAIN_MAX * 6;
    if (p_out_data.batchCache.buffer.size() < count) {
        p_out_data.batchCache.buffer.resize(count);
    }

    auto matrices = p_out_data.options.is_opengl ? math::BuildOpenGlCubeMapViewProjectionMatrix(Vector3f(0)) : BuildCubeMapViewProjectionMatrix(Vector3f(0));
    for (int mip_idx = 0; mip_idx < IBL_MIP_CHAIN_MAX; ++mip_idx) {
        for (int face_id = 0; face_id < 6; ++face_id) {
            auto& batch = p_out_data.batchCache.buffer[mip_idx * 6 + face_id];
            batch.c_cubeProjectionViewMatrix = matrices[face_id];
            batch.c_envPassRoughness = (float)mip_idx / (float)(IBL_MIP_CHAIN_MAX - 1);
        }
    }
}

auto Renderer::Impl::Initialize() -> Result<void> {
#if USING(USE_RENDERER_DEBUG)
    CommandRegistry& reg = m_app.CommandRegistry();
    reg.Register({
        .name = "render.pool.dump",
        .help = "List textures in transient pool.",
        .usage = "render.pool.dump",
        .fn = [this](CommandContext& p_ctx, const CommandArgs& p_args) {
            RenderPoolDump_Cmd(m_pool, p_ctx, p_args);
            return true;
        },
    });
#endif
    return Result<void>();
}

void Renderer::Impl::Tick(const FrameTime& p_frame, std::span<const ResolvedView> p_views) {
    CAVE_PROFILE_EVENT();

    auto submission = std::make_unique<RenderSubmission>();

    FramePlan plan = BuildFramePlan(p_frame, p_views);
    for (size_t idx = 0; idx < plan.frame_data.size(); ++idx) {
        const ResolvedView& view = plan.views[idx];
        const FrameData& data = plan.frame_data[idx];

        if (auto res = BuildRenderGraph(data.options, view); !res) {
            CRASH_NOW();
        } else {
            auto graph = *res;
            graph->Resolve(m_pool);

            submission->render_graph.push_back(graph);
        }
    }
    submission->frame_data = std::move(plan.frame_data);

    m_app.GetRenderDevice()->Submit(std::move(submission));
}

FramePlan Renderer::Impl::BuildFramePlan(const FrameTime& p_frame, std::span<const ResolvedView> p_views) {
    FramePlan plan;

    const bool is_opengl = m_app.IsOpenGL();
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

    plan.frame_data.resize(p_views.size());
    plan.views.reserve(p_views.size());

    int view_idx = 0;

    for (const ResolvedView& view : p_views) {
        RenderScene& render_scene = GetOrCreateRenderScene(view.scene_id);
        m_scene_builder.BuildFull(*view.scene, render_scene);

        plan.views.push_back(view);

        FrameData& framedata = plan.frame_data[view_idx];
        framedata.options = options;

        FillConstantBuffer(p_frame, view.scene, view, framedata);

        RunMeshRenderSystem(*view.scene, render_scene, view, framedata);
        RunTileMapRenderSystem(view.scene, framedata);
        RunSpriteRenderSystem(view.scene, framedata);
        RunDebugRenderSystem(view.scene, framedata);
        FillEnvConstants(framedata);

        // @HACK: only support first scene
        if (view_idx == 0) {
            RequestPathTracerUpdate(*view.scene);
        }

        ++view_idx;
    }

    return plan;
}

auto Renderer::Impl::BuildRenderGraph(const RenderOptions& p_plan,
                                      const ResolvedView& p_view) -> Result<std::shared_ptr<CompiledGraph>> {
    if (!IsPathTracerActive()) [[likely]] {
        return BuildRenderGraphDeferred(p_plan, p_view);
    }
    return BuildRenderGraphPathTracer(p_plan, p_view);
}

auto Renderer::Impl::BuildRenderGraphDeferred(const RenderOptions& p_plan,
                                              const ResolvedView& p_view) -> Result<std::shared_ptr<CompiledGraph>> {
    constexpr const char RG_RES_BRDF[] = "r:brdf";

    IRenderDevice& device = *m_app.GetRenderDevice();

    if (!m_brdf) {
        std::shared_ptr<ImageAsset> image = IAssetManager::GetSingleton().FindImage("brdf.hdr");
        m_brdf = device.CreateTexture(image.get());
    }
    if (!m_ltc1) {
        m_ltc1 = CreateLTC1(device);
    }
    if (!m_ltc2) {
        m_ltc2 = CreateLTC2(device);
    }

    RenderGraphBuilderExt graph(p_view.viewport_px);

    RGTextureId brdf = graph.ImportTexture({ m_brdf });
    RGTextureId ltc1 = graph.ImportTexture({ m_ltc1 });
    RGTextureId ltc2 = graph.ImportTexture({ m_ltc2 });

    auto env_outputs = m_env.Build(graph, p_plan);

    auto shadow_outputs = m_shadow.Build(graph, p_plan);

    // @TODO: refactor the following
    auto prepass_outputs = graph.AddDepthPrepass();

    auto gbuffer_outputs = graph.AddGbufferPass({
        .depth = prepass_outputs.depth,
    });

    SsaoFeature::Outputs ssao_outputs{};

    if (p_plan.enable_ssao) {
        SsaoFeature::Inputs ssao_inputs{
            .normal = gbuffer_outputs.color1,
            .depth = prepass_outputs.depth,
        };
        ssao_outputs = m_ssao.Build(graph, p_plan, ssao_inputs);
    }

    auto lighting_outputs = graph.AddLightingPass({
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

    auto forward_outputs = graph.AddForwardPass({
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

    auto highlight_outputs = graph.AddHighlightPass({
        .stencil = prepass_outputs.depth,
    });

    graph.AddPostProcessPass({
        .lighting = lighting_outputs.lighting,
        .outline = highlight_outputs.outline,
        .bloom = 0,
        .out = p_view.output,
    });

    return graph.Compile();
}

auto Renderer::Impl::BuildRenderGraphPathTracer(const RenderOptions& p_plan,
                                                const ResolvedView& p_view) -> Result<std::shared_ptr<CompiledGraph>> {
    RenderGraph graph(p_view.viewport_px);

    m_pt.Build(graph, p_plan, { p_view.output });

    return graph.Compile();
}

RenderScene& Renderer::Impl::GetOrCreateRenderScene(SceneId p_scene_id) {
    return m_scene_cache[p_scene_id];
}

}  // namespace cave::render
