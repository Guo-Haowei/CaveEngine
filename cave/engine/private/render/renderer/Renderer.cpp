#include "Renderer.h"

#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/render/features/EnvironmentFeature.h"
#include "engine/private/render/features/PrecomputedTextures.h"
#include "engine/private/render/features/ShadowFeature.h"
#include "engine/private/render/features/SsaoFeature.h"
#include "engine/private/render/renderer/FramePlan.h"
#include "engine/private/render/renderer/RendererDebug.h"
#include "engine/private/render/renderer/RenderScene.h"
#include "engine/private/render/renderer/RenderSceneBuilder.h"
#include "engine/private/render/renderer/RenderSubmission.h"
#include "engine/private/render/renderer/TransientPool.h"
#include "engine/private/render/render_graph/CompiledGraph.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/ISceneRegistry.h"

// @TODO: cleanup
#include "engine/private/core/base/random.h"
#include "engine/private/core/math/MatrixTransform.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/render/render_graph/CommonPasses.h"
#include "engine/private/render/render_graph/RenderGraphDefines.h"

// @TODO: remove
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/render/render_device/RenderDevice.h"

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

    void Tick(std::span<const ResolvedView> p_views);

private:
    FramePlan BuildFramePlan(std::span<const ResolvedView> p_views);
    auto BuildRenderGraph(const RenderOptions& p_plan,
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

void Renderer::Tick(std::span<const ResolvedView> p_views) {
    m_impl->Tick(p_views);
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
static void FillConstantBuffer(const Scene* p_scene,
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
        cache.c_cameraForward = (cam.view_inv * -Vector4f::UnitZ).xyz;
        cache.c_cameraRight = (cam.view_inv * Vector4f::UnitX).xyz;
        cache.c_cameraUp = (cam.view_inv * Vector4f::UnitY).xyz;
        cache.c_cameraPosition = (cam.view_inv * Vector4f::UnitW).xyz;
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

    // @TODO: refactor
    static int s_frameIndex = 0;
    cache.c_frameIndex = s_frameIndex++;
    // @TODO: fix this
    cache.c_sceneDirty = p_scene ? (p_scene->GetDirtyFlags() != SCENE_DIRTY_NONE) : true;

    // Force fields
    // int counter = 0;
    // for (auto [id, force_field_component] : p_scene.m_ForceFieldComponents) {
    //    ForceField& force_field = cache.c_forceFields[counter++];
    //    const TransformComponent& transform = *p_scene.GetComponent<TransformComponent>(id);
    //    force_field.position = transform.GetTranslation();
    //    force_field.strength = force_field_component.strength;
    //}

    // cache.c_forceFieldsCount = counter;
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
        },
    });
#endif
    return Result<void>();
}

void Renderer::Impl::Tick(std::span<const ResolvedView> p_views) {
    CAVE_PROFILE_EVENT();

    auto submission = std::make_unique<RenderSubmission>();

    FramePlan plan = BuildFramePlan(p_views);
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

FramePlan Renderer::Impl::BuildFramePlan(std::span<const ResolvedView> p_views) {
    FramePlan plan;

    const bool is_opengl = m_app.GetRenderDevice()->GetBackend() == Backend::OPENGL;
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

        FrameData& framedata = plan.frame_data[view_idx++];
        framedata.options = options;

        FillConstantBuffer(view.scene, view, framedata);

        RunMeshRenderSystem(*view.scene, render_scene, view, framedata);
        RunTileMapRenderSystem(view.scene, framedata);
        RunSpriteRenderSystem(view.scene, framedata);
        RunDebugRenderSystem(view.scene, framedata);
        FillEnvConstants(framedata);

        // @TODO: fix path tracer
        // if (p_scene) {
        //    RequestPathTracerUpdate(*camera, *p_scene);
        //}
        // if (ecs_scene) break;
    }

    return plan;
}

auto Renderer::Impl::BuildRenderGraph(const RenderOptions& p_plan, const ResolvedView& p_view) -> Result<std::shared_ptr<CompiledGraph>> {
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

    RenderGraphBuilderExt builder(p_view.viewport_px);

    RGTextureId brdf = builder.ImportTexture({ m_brdf });
    RGTextureId ltc1 = builder.ImportTexture({ m_ltc1 });
    RGTextureId ltc2 = builder.ImportTexture({ m_ltc2 });

    auto env_outputs = m_env.Build(builder, p_plan);

    auto shadow_outputs = m_shadow.Build(builder, p_plan);

    // @TODO: refactor the following
    auto prepass_outputs = builder.AddDepthPrepass();

    auto gbuffer_outputs = builder.AddGbufferPass({
        .depth = prepass_outputs.depth,
    });

    SsaoFeature::Outputs ssao_outputs{};

    if (p_plan.enable_ssao) {
        SsaoFeature::Inputs ssao_inputs{
            .normal = gbuffer_outputs.color1,
            .depth = prepass_outputs.depth,
        };
        ssao_outputs = m_ssao.Build(builder, p_plan, ssao_inputs);
    }

    auto lighting_outputs = builder.AddLightingPass({
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

    auto forward_outputs = builder.AddForwardPass({
        .skybox = env_outputs.ibl_diffuse,
        .shadow = shadow_outputs.shadow,
        .ibl_diffuse = env_outputs.ibl_diffuse,
        .ibl_prefiltered = env_outputs.ibl_prefiltered,
        .brdf = brdf,
        .ltc1 = ltc1,
        .ltc2 = ltc2,
        .depth = prepass_outputs.depth,
        .lighting = lighting_outputs.lighting,
    });

    auto highlight_outputs = builder.AddHighlightPass({
        .stencil = prepass_outputs.depth,
    });

    auto postprocess_outputs = builder.AddPostProcessPass({
        .lighting = lighting_outputs.lighting,
        .outline = highlight_outputs.outline,
        .bloom = 0,
        .out = p_view.output,
    });

    return builder.Compile();
}

RenderScene& Renderer::Impl::GetOrCreateRenderScene(SceneId p_scene_id) {
    return m_scene_cache[p_scene_id];
}

}  // namespace cave::render
