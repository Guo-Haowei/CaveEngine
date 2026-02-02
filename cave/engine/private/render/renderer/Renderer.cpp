#include "Renderer.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/debugger/Profiler.h"
#include "engine/private/render/features/EnvironmentFeature.h"
#include "engine/private/render/features/ShadowFeature.h"
#include "engine/private/render/renderer/FramePlan.h"
#include "engine/private/render/renderer/RenderScene.h"
#include "engine/private/render/renderer/RenderSceneBuilder.h"
#include "engine/private/render/renderer/RenderSubmission.h"
#include "engine/private/render/render_graph/RenderGraph.h"
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
#include "engine/private/renderer/ltc_matrix.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/render/render_device/RenderDevice.h"

namespace cave {

extern void RunTileMapRenderSystem(Scene* p_scene, FrameData& p_framedata);

extern void RunSpriteRenderSystem(const Scene* p_scene, FrameData& p_framedata);
extern void RunDebugRenderSystem(const Scene* p_scene, FrameData& p_framedata);

}  // namespace cave

// @HACK: expose render graph for debugging
cave::render::RenderGraph* g_graph = nullptr;

namespace cave::render {

using math::Vector2i;
using math::Vector3f;
using math::Vector4f;

// @TODO: refactor
static std::shared_ptr<GpuTexture> GenerateLTC(std::string_view p_name, const float* p_matrix_table) {
    constexpr int LTC_SIZE = 64;
    GpuTextureDesc desc{
        .type = AttachmentType::NONE,
        .dimension = Dimension::TEXTURE_2D,
        .width = LTC_SIZE,
        .height = LTC_SIZE,
        .depth = 1,
        .mipLevels = 1,
        .arraySize = 1,
        .format = PixelFormat::R32G32B32A32_FLOAT,
        .bindFlags = BIND_SHADER_RESOURCE,
        .miscFlags = RESOURCE_MISC_NONE,
        .initialData = p_matrix_table,
        .name = std::string(p_name),
    };

    return RenderDevice::GetSingleton().CreateTexture(desc, PointClampSampler());
}

class Renderer::Impl {
public:
    Impl(IApplication& p_app)
        : m_app(p_app) {}

    auto Initialize() -> Result<void>;

    void Tick(std::span<const render::ViewDesc> p_views);

private:
    FramePlan BuildFramePlan(std::span<const render::ViewDesc> p_views);
    auto BuildRenderGraph(const FramePlan& p_plan) -> Result<std::shared_ptr<RenderGraph>>;

    RenderScene& GetOrCreateRenderScene(SceneId p_scene_id);

private:
    IApplication& m_app;
    RenderSceneBuilder m_scene_builder;
    std::unordered_map<SceneId, RenderScene> m_scene_cache;

    // @TODO: remove
    std::shared_ptr<RenderGraph> m_render_graph;

    // features
    ShadowFeature m_shadow;
    EnvironmentFeature m_env;
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

void Renderer::Tick(std::span<const ViewDesc> p_views) {
    m_impl->Tick(p_views);
}

// @TODO: remove this
extern void RunMeshRenderSystem(const Scene& p_scene,
                                const RenderScene& p_rscene,
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

using KernelData = std::array<Vector4f, 64>;

static_assert(sizeof(KernelData) == sizeof(Vector4f) * SSAO_KERNEL_SIZE);

// @TODO: move it to somewhere else
static KernelData GenerateSsaoKernel() {
    auto lerp = [](float a, float b, float f) {
        return a + f * (b - a);
    };

    KernelData kernel;

    const int kernel_size = 32;
    const float inv_kernel_size = 1.0f / kernel_size;
    for (int i = 0; i < static_cast<int>(kernel.size()); ++i) {
        // [-1, 1], [-1, 1], [0, 1]
        Vector3f sample(Random::Float(-1.0f, 1.0f),
                        Random::Float(-1.0f, 1.0f),
                        Random::Float());

        sample = normalize(sample);
        sample *= Random::Float();
        float scale = i * inv_kernel_size;

        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        kernel[i].xyz = sample;
    }

    return kernel;
}

// @TODO: refactor
static void FillConstantBuffer(const Scene* p_scene, FrameData& p_out_data) {
    const auto& options = p_out_data.options;
    auto& cache = p_out_data.perFrameCache;

    // camera
    {
        const auto& cam = p_out_data.resolved_view;
        cache.c_camView = cam.view;
        cache.c_camProj = cam.proj;
        cache.c_invCamView = cam.view_inv;
        cache.c_invCamProj = cam.proj_inv;
        cache.c_cameraFovDegree = cam.fovy;
        cache.c_cameraForward = cam.front;
        cache.c_cameraRight = cam.right;
        cache.c_cameraUp = cam.up;
        cache.c_cameraPosition = cam.position;
    }

    // Bloom
    {
        cache.c_bloomThreshold = 1.3f;
        cache.c_enableBloom = options.bloomEnabled;

        cache.c_debugVoxelId = options.debugVoxelId;
        cache.c_ptObjectCount = p_scene ? ((int)p_scene->GetCount<MeshRendererComponent>()) : 0;
    }

    // IBL
    {
        cache.c_iblEnabled = options.iblEnabled;
    }

    // SSAO
    {
        // @TODO: do this properly
        static auto kernel_data = GenerateSsaoKernel();
        cache.c_ssaoEnabled = options.ssaoEnabled;
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

    auto matrices = p_out_data.options.isOpengl ? math::BuildOpenGlCubeMapViewProjectionMatrix(Vector3f(0)) : BuildCubeMapViewProjectionMatrix(Vector3f(0));
    for (int mip_idx = 0; mip_idx < IBL_MIP_CHAIN_MAX; ++mip_idx) {
        for (int face_id = 0; face_id < 6; ++face_id) {
            auto& batch = p_out_data.batchCache.buffer[mip_idx * 6 + face_id];
            batch.c_cubeProjectionViewMatrix = matrices[face_id];
            batch.c_envPassRoughness = (float)mip_idx / (float)(IBL_MIP_CHAIN_MAX - 1);
        }
    }
}

auto Renderer::Impl::Initialize() -> Result<void> {
    FramePlan dummy_plan;
    if (auto res = BuildRenderGraph(dummy_plan); !res) {
        return CAVE_ERROR(res.error());
    } else {
        m_render_graph = *res;

        g_graph = m_render_graph.get();
        return Result<void>();
    }
}

void Renderer::Impl::Tick(std::span<const render::ViewDesc> p_views) {
    CAVE_PROFILE_EVENT();

    auto submission = std::make_unique<RenderSubmission>();

    FramePlan plan = BuildFramePlan(p_views);

    submission->frame_data = std::move(plan.frame_data);
    submission->render_graph = m_render_graph;
    // submission->render_graph = BuildRenderGraph(plan);

    // @TODO: graph
    m_app.GetRenderDevice()->Submit(std::move(submission));
}

FramePlan Renderer::Impl::BuildFramePlan(std::span<const render::ViewDesc> p_views) {
    FramePlan plan;
    plan.frame_data.resize(p_views.size());

    const bool is_opengl = m_app.GetRenderDevice()->GetBackend() == Backend::OPENGL;
    RenderOptions options = {
        .isOpengl = is_opengl,
        .ssaoEnabled = DVAR_GET_BOOL(gfx_ssao_enabled),
        .vxgiEnabled = false,
        .bloomEnabled = DVAR_GET_BOOL(gfx_enable_bloom),
        .iblEnabled = DVAR_GET_BOOL(gfx_enable_ibl),
        .debugVoxelId = DVAR_GET_INT(gfx_debug_vxgi_voxel),
        .debugBvhDepth = DVAR_GET_INT(gfx_bvh_debug),
        .voxelTextureSize = DVAR_GET_INT(gfx_voxel_size),
        .ssaoKernelRadius = DVAR_GET_FLOAT(gfx_ssao_radius),
    };

    // @HACK: really need to refactor this crap
    if (plan.frame_data.size()) {
        static int s_should_bake = 0;
        if (m_app.GetStateId() != static_cast<AppStateId>(0)) {
            if (s_should_bake == 1) plan.frame_data[0].bakeIbl = true;
            ++s_should_bake;
        }
    }

    int i = 0;
    for (const render::ViewDesc& view : p_views) {
        Scene* ecs_scene = m_app.GetSceneRegistry()->Resolve(view.scene_id);
        DEV_ASSERT(ecs_scene);
        if (!ecs_scene) continue;

        RenderScene& render_scene = GetOrCreateRenderScene(view.scene_id);
        m_scene_builder.BuildFull(*ecs_scene, render_scene);

        ResolvedView resolved = ResolveView(view, ecs_scene, is_opengl);

        FrameData& framedata = plan.frame_data[i++];
        framedata.options = options;
        framedata.resolved_view = resolved;

        FillConstantBuffer(ecs_scene, framedata);

        RunMeshRenderSystem(*ecs_scene, render_scene, framedata);
        RunTileMapRenderSystem(ecs_scene, framedata);
        RunSpriteRenderSystem(ecs_scene, framedata);
        RunDebugRenderSystem(ecs_scene, framedata);
        FillEnvConstants(framedata);

        // @TODO: fix path tracer
        // if (p_scene) {
        //    RequestPathTracerUpdate(*camera, *p_scene);
        //}
        // if (ecs_scene) break;
    }

    return plan;
}

auto Renderer::Impl::BuildRenderGraph(const FramePlan& p_plan) -> Result<std::shared_ptr<RenderGraph>> {
    constexpr const char RG_RES_BRDF[] = "r:brdf";
    constexpr const char RG_RES_LTC1[] = "r:ltc1";
    constexpr const char RG_RES_LTC2[] = "r:ltc2";

    // @TODO: get frame size from viewport
    const Vector2i frame_size = DVAR_GET_IVEC2(resolution);
    RenderGraphBuilderConfig config;
    config.frameWidth = frame_size.x;
    config.frameHeight = frame_size.y;

    RenderGraphBuilderExt builder(config);

    RGTextureId brdf = builder.ImportTexture({
        .debug_name = RG_RES_BRDF,
        .func = []() {
            std::shared_ptr<ImageAsset> image = IAssetManager::GetSingleton().FindImage("brdf.hdr");
            return RenderDevice::GetSingleton().CreateTexture(image.get());
        },
    });

    RGTextureId ltc1 = builder.ImportTexture({
        .debug_name = RG_RES_LTC1,
        .func = []() { return GenerateLTC(RG_RES_LTC1, LTC1); },
    });

    RGTextureId ltc2 = builder.ImportTexture({
        .debug_name = RG_RES_LTC2,
        .func = []() { return GenerateLTC(RG_RES_LTC2, LTC2); },
    });

    auto env_outputs = m_env.Build(builder, p_plan);
    auto shadow_outputs = m_shadow.Build(builder, p_plan);

    // @TODO: refactor the following
    auto prepass_outputs = builder.AddDepthPrepass();

    auto gbuffer_outputs = builder.AddGbufferPass({
        .depth = prepass_outputs.depth,
    });

    SsaoOutput ssao_outputs{};
    if (p_plan.enable_ssao) {
        ssao_outputs = builder.AddSsaoPass({
            .depth = prepass_outputs.depth,
            .normal = gbuffer_outputs.color1,
        });
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
    });

    return builder.Compile();
}

RenderScene& Renderer::Impl::GetOrCreateRenderScene(SceneId p_scene_id) {
    return m_scene_cache[p_scene_id];
}

}  // namespace cave::render
