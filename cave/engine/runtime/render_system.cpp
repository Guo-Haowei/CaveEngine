#include "render_system.h"

#include "engine/core/base/random.h"
#include "engine/debugger/profiler.h"
#include "engine/math/matrix_transform.h"
#include "engine/render_graph/render_graph_defines.h"
#include "engine/renderer/frame_data.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/renderer/path_tracer_render_system.h"
#include "engine/runtime/application.h"
#include "engine/runtime/graphics_manager_interface.h"
#include "engine/scene/scene.h"

namespace cave {

// render systems
extern void RunMeshRenderSystem(Scene* p_scene, FrameData& p_framedata);
extern void RunTileMapRenderSystem(Scene* p_scene, FrameData& p_framedata);

extern void RunSpriteRenderSystem(const Scene* p_scene, FrameData& p_framedata);
extern void RunDebugRenderSystem(const Scene* p_scene, FrameData& p_framedata);

auto RenderSystem::InitializeImpl() -> Result<void> {
    return Result<void>();
}

void RenderSystem::FinalizeImpl() {
}

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

static void FillConstantBuffer(const Scene* p_scene, FrameData& p_out_data) {
    const auto& options = p_out_data.options;
    auto& cache = p_out_data.perFrameCache;

    // camera
    {
        const auto& view_info = p_out_data.view_info;
        cache.c_camView = view_info->view;
        cache.c_camProj = view_info->projection_rendering;
        cache.c_invCamView = glm::inverse(view_info->view);
        cache.c_invCamProj = glm::inverse(view_info->projection_rendering);
        cache.c_cameraFovDegree = view_info->fovy;
        cache.c_cameraForward = view_info->front;
        cache.c_cameraRight = view_info->right;
        cache.c_cameraUp = view_info->up;
        cache.c_cameraPosition = view_info->position;
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

    auto matrices = p_out_data.options.isOpengl ? BuildOpenGlCubeMapViewProjectionMatrix(Vector3f(0)) : BuildCubeMapViewProjectionMatrix(Vector3f(0));
    for (int mip_idx = 0; mip_idx < IBL_MIP_CHAIN_MAX; ++mip_idx) {
        for (int face_id = 0; face_id < 6; ++face_id) {
            auto& batch = p_out_data.batchCache.buffer[mip_idx * 6 + face_id];
            batch.c_cubeProjectionViewMatrix = matrices[face_id];
            batch.c_envPassRoughness = (float)mip_idx / (float)(IBL_MIP_CHAIN_MAX - 1);
        }
    }
}

void RenderSystem::BeginFrame() {
    if (m_frameData) {
        delete m_frameData;
        m_frameData = nullptr;
    }

    RenderOptions options = {
        .isOpengl = m_app->GetGraphicsManager()->GetBackend() == Backend::OPENGL,
        .ssaoEnabled = DVAR_GET_BOOL(gfx_ssao_enabled),
        .vxgiEnabled = false,
        .bloomEnabled = DVAR_GET_BOOL(gfx_enable_bloom),
        .iblEnabled = DVAR_GET_BOOL(gfx_enable_ibl),
        .debugVoxelId = DVAR_GET_INT(gfx_debug_vxgi_voxel),
        .debugBvhDepth = DVAR_GET_INT(gfx_bvh_debug),
        .voxelTextureSize = DVAR_GET_INT(gfx_voxel_size),
        .ssaoKernelRadius = DVAR_GET_FLOAT(gfx_ssao_radius),
    };

    // @HACK: really need to
    m_frameData = new FrameData(options);
    static int s_should_bake = 0;
    if (m_app->GetStateId() != static_cast<AppStateId>(0)) {
        if (s_should_bake == 1) m_frameData->bakeIbl = true;
        ++s_should_bake;
    }
}

void RenderSystem::RenderFrame(std::vector<SceneView>& p_views) {
    // HACK
    auto backend = m_app->GetGraphicsManager()->GetBackend();
    switch (backend) {
        case cave::Backend::OPENGL:
        case cave::Backend::D3D11:
        case cave::Backend::D3D12:
            break;
        default:
            return;
    }

    CAVE_PROFILE_EVENT();

    DEV_ASSERT(m_frameData);
    FrameData& framedata = *m_frameData;

    for (SceneView& view : p_views) {
        Scene* p_scene = view.scene;
        // @TODO: only support one view, fix this
        framedata.view_info = &view.view_info;

        FillConstantBuffer(p_scene, framedata);
        RunMeshRenderSystem(p_scene, framedata);
        RunTileMapRenderSystem(p_scene, framedata);
        RunSpriteRenderSystem(p_scene, framedata);
        RunDebugRenderSystem(p_scene, framedata);
        FillEnvConstants(framedata);

        // @TODO: fix path tracer
        // if (p_scene) {
        //    RequestPathTracerUpdate(*camera, *p_scene);
        //}
    }
}

}  // namespace cave
