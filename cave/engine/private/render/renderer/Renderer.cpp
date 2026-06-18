#include "Renderer.h"

#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/framework/IUIRuntime.h"

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

extern void RunTileMapRenderSystem(Scene* p_scene, FrameData& p_framedata);

extern void RunSpriteRenderSystem(const Scene* p_scene, FrameData& p_framedata);
extern void RunDebugRenderSystem(const Scene* p_scene, FrameData& p_framedata);

}  // namespace cave

namespace cave::render {

using math::Vec2f;
using math::Vec2i;
using math::Vec3f;
using math::Vec4f;

class Renderer::Impl {
public:
    Impl(IRenderDevice& device)
        : device_(device)
        , transient_pool_(device)
        , env_(transient_pool_, device)
        , ssao_(device) {}

    void tick(const FrameTime& time,
              std::span<const ResolvedView> views,
              const UIFrameDrawData& ui_draw_data);

    void setMode(bool is_2d) { is_2d_ = is_2d; }

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

    void createOrUpdateUIBuffers(const BuiltUIData& ui_data);

private:
    IRenderDevice& device_;
    RenderSceneBuilder m_scene_builder;
    std::unordered_map<SceneId, RenderScene> scene_cache_;

    // features
    TransientPool transient_pool_;
    EnvironmentFeature env_;
    ShadowFeature shadow_;
    SsaoFeature ssao_;
    PathTracerFeature pathtracer_;

    GpuTextureId brdf_{};
    GpuTextureId ltc1_{};
    GpuTextureId ltc2_{};
    bool is_2d_{ false };

    std::shared_ptr<GpuMesh> ui_buffers_;
};

Renderer::Renderer(IRenderDevice& device)
    : impl_(std::make_unique<Impl>(device)) {}

Renderer::~Renderer() = default;

void Renderer::tick(const FrameTime& p_frame,
                    std::span<const ResolvedView> p_views,
                    const UIFrameDrawData& p_ui_data) {
    impl_->tick(p_frame, p_views, p_ui_data);
}

void Renderer::setMode(bool is_2d) {
    impl_->setMode(is_2d);
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

static bool updateAllUIBuffer(IRenderDevice& p_device,
                              const BuiltUIData& p_data,
                              GpuMesh& p_mesh) {
    if (!updateUIBuffer(p_device, p_data.indices, p_mesh.indexBuffer.get()))
        return false;
    if (!updateUIBuffer(p_device, p_data.positions, p_mesh.vertexBuffers.at(0).get()))
        return false;
    if (!updateUIBuffer(p_device, p_data.colors, p_mesh.vertexBuffers.at(1).get()))
        return false;
    return true;
}

// @TODO: consider move to UIRenderer
void Renderer::Impl::createOrUpdateUIBuffers(const BuiltUIData& ui_data) {
    if (ui_buffers_) {
        if (!updateAllUIBuffer(device_, ui_data, *ui_buffers_)) {
            // @TODO: proper error handling
            CRASH_NOW_MSG("Failed to update UI buffer");
        }
        // @TODO: if failed to update buffer, create a new one
        return;
    }

    std::array<GpuBufferDesc, 2> vb_desc{};
    vb_desc[0] = fillDesc(ui_data.positions);
    vb_desc[1] = fillDesc(ui_data.colors);

    GpuBufferDesc ib_desc = fillDesc(ui_data.indices);
    ib_desc.type = GpuBufferType::Index;

    GpuMeshDesc mesh_desc{};
    mesh_desc.drawCount = ib_desc.element_count;
    mesh_desc.enabledVertexCount = (uint32_t)vb_desc.size();
    mesh_desc.vertexLayout[0] = GpuMeshDesc::VertexLayout{ 0, sizeof(Vec2f), 0 };
    mesh_desc.vertexLayout[1] = GpuMeshDesc::VertexLayout{ 1, sizeof(Color), 0 };

    ui_buffers_ = device_.CreateMeshImpl(mesh_desc, vb_desc, &ib_desc).value();
}

void Renderer::Impl::tick(const FrameTime& time,
                          std::span<const ResolvedView> views,
                          const UIFrameDrawData& ui_draw_data) {
    CAVE_PROFILE_EVENT();

    auto submission = std::make_unique<RenderSubmission>();

    FramePlan plan = buildFramePlan(time, views);

    const BuiltUIData ui_data = BuildUIData(views, ui_draw_data);
    DEV_ASSERT(ui_data.batches.size() == views.size());
    if (!ui_data.indices.empty()) {
        createOrUpdateUIBuffers(ui_data);
    }

    for (size_t idx = 0; idx < plan.frame_data.size(); ++idx) {
        const ResolvedView& view = plan.views[idx];
        FrameData& data = plan.frame_data[idx];
        data.ui_batch = ui_data.batches[idx];
        data.ui_buffer = ui_buffers_;

        if (auto res = buildRenderGraph(data.options, view); !res) {
            CRASH_NOW();
        } else {
            auto graph = *res;
            graph->Resolve(transient_pool_);

            submission->render_graph.push_back(graph);
        }
    }

    submission->frame_data = std::move(plan.frame_data);

    device_.submit(std::move(submission));
}

FramePlan Renderer::Impl::buildFramePlan(const FrameTime& time,
                                         std::span<const ResolvedView> views) {
    FramePlan plan;

    const bool is_opengl = device_.backend() == rhi::Backend::OpenGL;
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
        m_scene_builder.BuildFull(*view.scene, render_scene);

        plan.views.push_back(view);

        FrameData& framedata = plan.frame_data[view_idx];
        framedata.options = options;

        fillConstantBuffer(time, view.scene, view, framedata);

        runMeshRenderSystem(*view.scene, render_scene, view, framedata);
        RunTileMapRenderSystem(view.scene, framedata);
        RunSpriteRenderSystem(view.scene, framedata);
        RunDebugRenderSystem(view.scene, framedata);
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

    return is_2d_ ? buildRenderGraph2d(plan, view) : buildRenderGraphDeferred(plan, view);
}

auto Renderer::Impl::buildRenderGraphDeferred(const RenderOptions& plan_,
                                              const ResolvedView& view_) -> Result<std::shared_ptr<CompiledGraph>> {
    if (!brdf_) {
        std::shared_ptr<ImageAsset> image = IAssetManager::singleton().FindImage("brdf.hdr");
        brdf_ = device_.CreateTexture(image.get());
    }
    if (!ltc1_) {
        ltc1_ = CreateLTC1(device_);
    }
    if (!ltc2_) {
        ltc2_ = CreateLTC2(device_);
    }

    RenderGraphBuilderExt graph(view_.viewport_px);

    RGTextureId brdf = graph.ImportTexture({ brdf_ });
    RGTextureId ltc1 = graph.ImportTexture({ ltc1_ });
    RGTextureId ltc2 = graph.ImportTexture({ ltc2_ });

    auto env_outputs = env_.Build(graph, plan_);

    auto shadow_outputs = shadow_.Build(graph, plan_);

    // @TODO: refactor the following
    auto prepass_outputs = graph.addDepthPrepass();

    auto gbuffer_outputs = graph.addGbufferPass({
        .depth = prepass_outputs.depth,
    });

    SsaoFeature::Outputs ssao_outputs{};

    if (plan_.enable_ssao) {
        SsaoFeature::Inputs ssao_inputs{
            .normal = gbuffer_outputs.color1,
            .depth = prepass_outputs.depth,
        };
        ssao_outputs = ssao_.Build(graph, plan_, ssao_inputs);
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
        .color_attachment = view_.output,
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
    return impl_->Cmd_dump(ctx, args);
}

bool Renderer::Impl::Cmd_dump(CommandContext& ctx, const CommandArgs& args) {
    RenderPoolDump_Cmd(transient_pool_, ctx, args);
    return true;
}
#endif

}  // namespace cave::render
