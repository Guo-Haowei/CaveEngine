#include "PipelineStateManager.h"

#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/render/rhi/PipelineStateObjects.h"
#include "engine/private/render/render_graph/RenderGraphDefines.h"

namespace cave::render {

#define CAVE_VXGI         NOT_IN_USE
#define CAVE_PARTICLE     NOT_IN_USE
#define CAVE_PATH_TRACER  NOT_IN_USE
#define CAVE_POINT_SHADOW NOT_IN_USE

const BlendDesc& PipelineStateManager::defaultBlendDesc() {
    return s_default_blend_state;
}

const BlendDesc& PipelineStateManager::blendDescDisabled() {
    return s_blend_state_off;
}

PipelineState* PipelineStateManager::findPSO(PipelineStateName p_name) {
    DEV_ASSERT_INDEX(p_name, m_pso_cache.size());
    return m_pso_cache[p_name].get();
}

auto PipelineStateManager::create(PipelineStateName p_name, const PipelineStateDesc& p_desc) -> Result<void> {
    if (p_desc.cs.empty()) {
        DEV_ASSERT(p_desc.depth_stencil_desc);
    }

    ERR_FAIL_COND_V(m_pso_cache[p_name] != nullptr, CAVE_ERROR(ErrorCode::ERR_ALREADY_EXISTS, "pipeline already exists"));

    Owner<PipelineState> pipeline{};
    switch (p_desc.type) {
        case PipelineStateType::GRAPHICS: {
            DEV_ASSERT(!p_desc.vs.empty());
            DEV_ASSERT(p_desc.rasterizer_desc);
            DEV_ASSERT(p_desc.depth_stencil_desc);
            DEV_ASSERT(p_desc.blend_desc);
            auto result = graphicsPipeline(p_desc);
            if (!result) {
                return CAVE_ERROR(result.error());
            }
            pipeline = std::move(*result);
        } break;
        case PipelineStateType::COMPUTE: {
            DEV_ASSERT(!p_desc.cs.empty());
            auto result = computePipeline(p_desc);
            if (!result) {
                return CAVE_ERROR(result.error());
            }
            pipeline = std::move(*result);
        } break;
        default:
            CRASH_NOW();
            break;
    }

    if (pipeline == nullptr) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "failed to create pipeline '{}'", EnumToString(p_name));
    }

    m_pso_cache[p_name] = std::move(pipeline);
    return Result<void>();
}

Result<void> PipelineStateManager::initialize(const RenderCapabilities& capabilities) {
    if constexpr (USING(PLATFORM_WASM)) {
        return Result<void>();
    }

    switch (m_backend) {
        case Backend::Null:
        case Backend::Direct3D12:
        case Backend::Vulkan:
            return Result<void>();
        default:
            break;
    }

#define CREATE_PSO(...)                                                           \
    do {                                                                          \
        if (auto res = create(__VA_ARGS__); !res) return CAVE_ERROR(res.error()); \
    } while (0)

    // @TODO: merge primitive and overlay
    CREATE_PSO(PSO_PRIMITIVE,
               {
                   .vs = "primitive.vs",
                   .ps = "primitive.ps",
                   .rasterizer_desc = &s_rasterizer_double_sided,
                   .depth_stencil_desc = &s_depth_stencil_off,
                   .input_layout_desc = &s_input_layout_primitive,
                   .blend_desc = &s_transparent,
                   .num_render_targets = 1,
                   .rtv_formats = { RT_FMT_TONE },
                   .dsv_format = PixelFormat::D32_FLOAT_S8X24_UINT,  // gbuffer
               });

    CREATE_PSO(PSO_UI_OVERLAY, {
                                   .vs = "ui_overlay.vs",
                                   .ps = "primitive.ps",
                                   .rasterizer_desc = &s_rasterizer_double_sided,
                                   .depth_stencil_desc = &s_depth_stencil_off,
                                   .input_layout_desc = &s_input_layout_primitive,
                                   .blend_desc = &s_transparent,
                                   .num_render_targets = 1,
                                   .rtv_formats = { RT_FMT_TONE },
                                   .dsv_format = {},
                               });

    CREATE_PSO(PSO_PREPASS,
               {
                   .vs = "mesh.vs",
                   .rasterizer_desc = &s_rasterizer_cull_back,
                   .depth_stencil_desc = &s_depth_reversed_stencil_on,
                   .input_layout_desc = &s_input_layout_mesh,
                   .blend_desc = &s_default_blend_state,
                   .num_render_targets = 0,
                   .rtv_formats = {},
                   .dsv_format = PixelFormat::D32_FLOAT_S8X24_UINT,  // gbuffer
               });

    CREATE_PSO(PSO_GBUFFER,
               {
                   .vs = "mesh.vs",
                   .ps = "gbuffer.ps",
                   .rasterizer_desc = &s_rasterizer_cull_back,
                   .depth_stencil_desc = &s_depth_reversed_stencil_off,
                   .input_layout_desc = &s_input_layout_mesh,
                   .blend_desc = &s_default_blend_state,
                   .num_render_targets = 3,
                   .rtv_formats = { RT_FMT_GBUFFER_BASE_COLOR,
                                    RT_FMT_GBUFFER_NORMAL,
                                    RT_FMT_GBUFFER_MATERIAL },
                   .dsv_format = PixelFormat::D32_FLOAT_S8X24_UINT,  // gbuffer
               });

    CREATE_PSO(PSO_GBUFFER_DOUBLE_SIDED,
               {
                   .vs = "mesh.vs",
                   .ps = "gbuffer.ps",
                   .rasterizer_desc = &s_rasterizer_double_sided,
                   .depth_stencil_desc = &s_depth_reversed_stencil_off,
                   .input_layout_desc = &s_input_layout_mesh,
                   .blend_desc = &s_default_blend_state,
                   .num_render_targets = 3,
                   .rtv_formats = { RT_FMT_GBUFFER_BASE_COLOR,
                                    RT_FMT_GBUFFER_NORMAL,
                                    RT_FMT_GBUFFER_MATERIAL },
                   .dsv_format = PixelFormat::D32_FLOAT_S8X24_UINT,  // gbuffer
               });

    CREATE_PSO(PSO_FORWARD_TRANSPARENT,
               {
                   .vs = "mesh.vs",
                   .ps = "forward.ps",
                   .rasterizer_desc = &s_rasterizer_double_sided,
                   .depth_stencil_desc = &s_depth_reversed_stencil_off,
                   .input_layout_desc = &s_input_layout_mesh,
                   .blend_desc = &s_transparent,
                   .num_render_targets = 1,
                   .rtv_formats = { RT_FMT_LIGHTING },
                   .dsv_format = PixelFormat::D32_FLOAT_S8X24_UINT,  // gbuffer
               });

    CREATE_PSO(PSO_DPETH, {
                              .vs = "shadow.vs",
                              .ps = "depth.ps",
                              .rasterizer_desc = &s_rasterizer_cull_front,
                              .depth_stencil_desc = &s_default_depth_stencil,
                              .input_layout_desc = &s_input_layout_mesh,
                              .blend_desc = &s_default_blend_state,
                              .num_render_targets = 0,
                              .dsv_format = PixelFormat::D32_FLOAT,
                          });

    CREATE_PSO(PSO_LIGHTING, {
                                 .vs = "screenspace_quad.vs",
                                 .ps = "lighting.ps",
                                 .rasterizer_desc = &s_rasterizer_cull_back,
                                 .depth_stencil_desc = &s_depth_stencil_off,
                                 .blend_desc = &s_default_blend_state,
                                 .num_render_targets = 1,
                                 .rtv_formats = { RT_FMT_LIGHTING },
                             });

    CREATE_PSO(PSO_HIGHLIGHT, {
                                  .vs = "screenspace_quad.vs",
                                  .ps = "highlight.ps",
                                  .rasterizer_desc = &s_rasterizer_cull_back,
                                  .depth_stencil_desc = &s_depth_reversed_stencil_on_highlight,
                                  .blend_desc = &s_default_blend_state,
                                  .num_render_targets = 1,
                                  .rtv_formats = { RT_FMT_OUTLINE_SELECT },
                                  .dsv_format = PixelFormat::D32_FLOAT_S8X24_UINT,  // gbuffer
                              });

    CREATE_PSO(PSO_SSAO, {
                             .vs = "screenspace_quad.vs",
                             .ps = "ssao.ps",
                             .rasterizer_desc = &s_rasterizer_cull_back,
                             .depth_stencil_desc = &s_depth_stencil_off,
                             .blend_desc = &s_default_blend_state,
                             .num_render_targets = 1,
                             .rtv_formats = { RT_FMT_SSAO },
                         });

    CREATE_PSO(PSO_POST_PROCESS, {
                                     .vs = "screenspace_quad.vs",
                                     .ps = "post_process.ps",
                                     .rasterizer_desc = &s_rasterizer_cull_back,
                                     .depth_stencil_desc = &s_depth_stencil_off,
                                     .blend_desc = &s_default_blend_state,
                                     .num_render_targets = 1,
                                     .rtv_formats = { RT_FMT_TONE },
                                 });

    if (capabilities.supportComputeShaders) {
        CREATE_PSO(PSO_BLOOM_SETUP, { .type = PipelineStateType::COMPUTE, .cs = "bloom_setup.cs" });
        CREATE_PSO(PSO_BLOOM_DOWNSAMPLE, { .type = PipelineStateType::COMPUTE, .cs = "bloom_downsample.cs" });
        CREATE_PSO(PSO_BLOOM_UPSAMPLE, { .type = PipelineStateType::COMPUTE, .cs = "bloom_upsample.cs" });
    }

    CREATE_PSO(PSO_ENV_SKYBOX, {
                                   .vs = "skybox.vs",
                                   .ps = "skybox.ps",
                                   .rasterizer_desc = &s_rasterizer_cull_back,
                                   .depth_stencil_desc = &s_skybox_depth_stencil,
                                   .input_layout_desc = &s_input_layout_mesh,
                                   .blend_desc = &s_default_blend_state,
                                   .num_render_targets = 1,
                                   .rtv_formats = { RT_FMT_LIGHTING },
                                   .dsv_format = PixelFormat::D32_FLOAT_S8X24_UINT,
                               });

    if constexpr (1) {
        CREATE_PSO(PSO_ENV_SKYBOX_TO_CUBE_MAP, {
                                                   .vs = "cube_map.vs",
                                                   .ps = "to_cube_map.ps",
                                                   .rasterizer_desc = &s_rasterizer_cull_back,
                                                   .depth_stencil_desc = &s_depth_stencil_off,
                                                   .input_layout_desc = &s_input_layout_mesh,
                                                   .blend_desc = &s_default_blend_state,
                                                   .num_render_targets = 1,
                                                   .rtv_formats = { PixelFormat::R32G32B32A32_FLOAT },
                                               });

        CREATE_PSO(PSO_DIFFUSE_IRRADIANCE, {
                                               .vs = "cube_map.vs",
                                               .ps = "diffuse_irradiance.ps",
                                               .rasterizer_desc = &s_rasterizer_cull_back,
                                               .depth_stencil_desc = &s_depth_stencil_off,
                                               .input_layout_desc = &s_input_layout_mesh,
                                               .blend_desc = &s_default_blend_state,
                                               .num_render_targets = 1,
                                               .rtv_formats = { PixelFormat::R32G32B32A32_FLOAT },
                                           });

        CREATE_PSO(PSO_PREFILTER, {
                                      .vs = "cube_map.vs",
                                      .ps = "prefilter.ps",
                                      .rasterizer_desc = &s_rasterizer_cull_back,
                                      .depth_stencil_desc = &s_depth_stencil_off,
                                      .input_layout_desc = &s_input_layout_mesh,
                                      .blend_desc = &s_default_blend_state,
                                      .num_render_targets = 1,
                                      .rtv_formats = { PixelFormat::R32G32B32A32_FLOAT },
                                  });
    }

    if (capabilities.supportComputeShaders) {
        CREATE_PSO(PSO_PATH_TRACER, { .type = PipelineStateType::COMPUTE, .cs = "path_tracer.cs" });
    }

#if USING(CAVE_PARTICLE)
    CREATE_PSO(PSO_PARTICLE_INIT, { .type = PipelineStateType::COMPUTE, .cs = "particle_initialization.cs" });
    CREATE_PSO(PSO_PARTICLE_KICKOFF, { .type = PipelineStateType::COMPUTE, .cs = "particle_kickoff.cs" });
    CREATE_PSO(PSO_PARTICLE_EMIT, { .type = PipelineStateType::COMPUTE, .cs = "particle_emission.cs" });
    CREATE_PSO(PSO_PARTICLE_SIM, { .type = PipelineStateType::COMPUTE, .cs = "particle_simulation.cs" });
    CREATE_PSO(PSO_PARTICLE_RENDERING, {
                                           .vs = "particle_draw.vs",
                                           .ps = "particle_draw.ps",
                                           .rasterizer_desc = &s_rasterizer_double_sided,
                                           .depth_stencil_desc = &s_depth_reversed_stencil_off,
                                           .input_layout_desc = &s_input_layout_mesh,
                                           .blend_desc = &s_transparent,
                                           .num_render_targets = 1,
                                           .rtv_formats = { RT_FMT_LIGHTING },
                                           .dsv_format = PixelFormat::D32_FLOAT_S8X24_UINT,  // gbuffer
                                       });
#endif

#if USING(CAVE_POINT_SHADOW)
    CREATE_PSO(PSO_POINT_SHADOW, {
                                     .vs = "shadowmap_point.vs",
                                     .ps = "shadowmap_point.ps",
                                     .rasterizer_desc = &s_rasterizer_cull_front,
                                     .depth_stencil_desc = &s_default_depth_stencil,
                                     .input_layout_desc = &s_input_layout_mesh,
                                     .blend_desc = &s_default_blend_state,
                                     .num_render_targets = 0,
                                     .dsv_format = PixelFormat::D32_FLOAT,
                                 });
#endif

#if USING(CAVE_VXGI)
    CREATE_PSO(PSO_VOXELIZATION_PRE, { .type = PipelineStateType::COMPUTE, .cs = "voxelization_pre.cs" });
    CREATE_PSO(PSO_VOXELIZATION_POST, { .type = PipelineStateType::COMPUTE, .cs = "voxelization_post.cs" });

    CREATE_PSO(PSO_VOXELIZATION, {
                                     .vs = "voxelization.vs",
                                     .ps = "voxelization.ps",
                                     .gs = "voxelization.gs",
                                     .rasterizer_desc = &s_rasterizer_double_sided,
                                     .depth_stencil_desc = &s_depth_stencil_off,
                                     .blend_desc = &s_blend_state_off,
                                 });

    CREATE_PSO(PSO_DEBUG_VOXEL, {
                                    .vs = "visualization.vs",
                                    .ps = "visualization.ps",
                                    .rasterizer_desc = &s_rasterizer_cull_back,
                                    .depth_stencil_desc = &s_depth_reversed_stencil_off,
                                    .blend_desc = &s_default_blend_state,
                                });
#endif

#undef CREATE_PSO

    return Result<void>();
}

void PipelineStateManager::finalize() {
    for (size_t idx = 0; idx < m_pso_cache.size(); ++idx) {
        m_pso_cache[idx].reset();
    }
}

}  // namespace cave::render
