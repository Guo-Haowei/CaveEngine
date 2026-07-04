#include "PipelineStateManager.h"

#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/render/rhi/PipelineStateObjects.h"
#include "engine/private/render/render_graph/RenderGraphDefines.h"

namespace cave::render {

const BlendDesc& PipelineStateManager::defaultBlendDesc() {
    return s_default_blend_state;
}

const BlendDesc& PipelineStateManager::blendDescDisabled() {
    return s_blend_state_off;
}

PipelineState* PipelineStateManager::findPSO(PipelineStateName p_name) {
    DEV_ASSERT_INDEX(p_name, pso_cache_.size());
    return pso_cache_[p_name].get();
}

auto PipelineStateManager::create(PipelineStateName p_name, const PipelineStateDesc& p_desc) -> Result<void> {
    if (p_desc.cs.empty()) {
        DEV_ASSERT(p_desc.depth_stencil_desc);
    }

    ERR_FAIL_COND_V(pso_cache_[p_name] != nullptr, CAVE_ERROR(ErrorCode::ERR_ALREADY_EXISTS, "pipeline already exists"));

    std::shared_ptr<PipelineState> pipeline{};
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
            pipeline = *result;
        } break;
        case PipelineStateType::COMPUTE: {
            DEV_ASSERT(!p_desc.cs.empty());
            auto result = computePipeline(p_desc);
            if (!result) {
                return CAVE_ERROR(result.error());
            }
            pipeline = *result;
        } break;
        default:
            CRASH_NOW();
            break;
    }

    if (pipeline == nullptr) {
        return CAVE_ERROR(ErrorCode::ERR_CANT_CREATE, "failed to create pipeline '{}'", EnumToString(p_name));
    }

    pso_cache_[p_name] = pipeline;
    return Result<void>();
}

Result<void> PipelineStateManager::initialize() {
    if constexpr (USING(PLATFORM_WASM)) {
        return Result<void>();
    }
    switch (backend_) {
        case Backend::Null:
        case Backend::Direct3D12:
        case Backend::Metal:
        case Backend::Vulkan:
            return Result<void>();
        default:
            break;
    }

#define CREATE_PSO(...)                                                           \
    do {                                                                          \
        if (auto res = create(__VA_ARGS__); !res) return CAVE_ERROR(res.error()); \
    } while (0)

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
                   .num_render_targets = 4,
                   .rtv_formats = { RT_FMT_GBUFFER_BASE_COLOR,
                                    RT_FMT_GBUFFER_POSITION,
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
                   .num_render_targets = 4,
                   .rtv_formats = { RT_FMT_GBUFFER_BASE_COLOR,
                                    RT_FMT_GBUFFER_POSITION,
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
                                 .dsv_format = PixelFormat::D32_FLOAT_S8X24_UINT,
                             });

#pragma region PSO_PARTICLE
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
#pragma endregion PSO_PARTICLE

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
                                     .dsv_format = PixelFormat::D32_FLOAT_S8X24_UINT,  // gbuffer
                                 });

    CREATE_PSO(PSO_UI_OVERLAY, {
                                   .vs = "ui_overlay.vs",
                                   .ps = "ui_overlay.ps",
                                   .rasterizer_desc = &s_rasterizer_double_sided,
                                   .depth_stencil_desc = &s_depth_stencil_off,
                                   .input_layout_desc = &s_input_layout_ui,
                                   .blend_desc = &s_transparent,
                                   .num_render_targets = 1,
                                   .rtv_formats = { RT_FMT_TONE },
                                   .dsv_format = PixelFormat::D32_FLOAT_S8X24_UINT,
                               });

#pragma region PSO_BLOOM
    CREATE_PSO(PSO_BLOOM_SETUP, { .type = PipelineStateType::COMPUTE, .cs = "bloom_setup.cs" });
    CREATE_PSO(PSO_BLOOM_DOWNSAMPLE, { .type = PipelineStateType::COMPUTE, .cs = "bloom_downsample.cs" });
    CREATE_PSO(PSO_BLOOM_UPSAMPLE, { .type = PipelineStateType::COMPUTE, .cs = "bloom_upsample.cs" });
#pragma endregion PSO_BLOOM

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

#pragma region PSO_ENV
    CREATE_PSO(PSO_ENV_SKYBOX_TO_CUBE_MAP, {
                                               .vs = "cube_map.vs",
                                               .ps = "to_cube_map.ps",
                                               .rasterizer_desc = &s_rasterizer_cull_back,
                                               .depth_stencil_desc = &s_default_depth_stencil,
                                               .input_layout_desc = &s_input_layout_mesh,
                                               .blend_desc = &s_default_blend_state,
                                           });

    CREATE_PSO(PSO_DIFFUSE_IRRADIANCE, {
                                           .vs = "cube_map.vs",
                                           .ps = "diffuse_irradiance.ps",
                                           .rasterizer_desc = &s_rasterizer_cull_back,
                                           .depth_stencil_desc = &s_default_depth_stencil,
                                           .input_layout_desc = &s_input_layout_mesh,
                                           .blend_desc = &s_default_blend_state,
                                       });

    CREATE_PSO(PSO_PREFILTER, {
                                  .vs = "cube_map.vs",
                                  .ps = "prefilter.ps",
                                  .rasterizer_desc = &s_rasterizer_cull_back,
                                  .depth_stencil_desc = &s_default_depth_stencil,
                                  .input_layout_desc = &s_input_layout_mesh,
                                  .blend_desc = &s_default_blend_state,
                              });
#pragma endregion PSO_ENV

    CREATE_PSO(PSO_SPRITE,
               {
                   .vs = "sprite.vs",
                   .ps = "sprite.ps",
                   .rasterizer_desc = &s_rasterizer_double_sided,
                   .depth_stencil_desc = &s_depth_reversed_stencil_off,
                   .input_layout_desc = &s_input_layout_sprite,
                   .blend_desc = &s_transparent,
                   .num_render_targets = 1,
                   .rtv_formats = { RT_FMT_TONE },
                   .dsv_format = PixelFormat::D32_FLOAT_S8X24_UINT,  // gbuffer
               });

    CREATE_PSO(PSO_SPRITE_NO_VERT,
               {
                   .vs = "sprite_no_vert.vs",
                   .ps = "sprite.ps",
                   .rasterizer_desc = &s_rasterizer_double_sided,
                   .depth_stencil_desc = &s_depth_reversed_stencil_off,
                   .blend_desc = &s_transparent,
                   .num_render_targets = 1,
                   .rtv_formats = { RT_FMT_TONE },
                   .dsv_format = PixelFormat::D32_FLOAT_S8X24_UINT,  // gbuffer
               });

    CREATE_PSO(PSO_DEBUG_DRAW,
               {
                   .vs = "debug_draw.vs",
                   .ps = "debug_draw.ps",
                   .rasterizer_desc = &s_rasterizer_double_sided,
                   .depth_stencil_desc = &s_depth_reversed_stencil_off,
                   .input_layout_desc = &s_input_layout_debug,
                   .blend_desc = &s_transparent,
                   .num_render_targets = 1,
                   .rtv_formats = { RT_FMT_TONE },
                   .dsv_format = PixelFormat::D32_FLOAT_S8X24_UINT,  // gbuffer
               });
    CREATE_PSO(PSO_PATH_TRACER, { .type = PipelineStateType::COMPUTE, .cs = "path_tracer.cs" });

    // @HACK: only support this many shaders
    if (backend_ != Backend::OpenGL) {
        return Result<void>();
    }

#pragma region PSO_VOXEL
    // Voxel
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
#pragma endregion PSO_VOXEL

#if 0
    CREATE_PSO(PSO_BILLBOARD, {
                                  .vs = "billboard.vs",
                                  .ps = "texture.ps",
                                  .rasterizer_desc = &s_rasterizer_double_sided,
                                  .depth_stencil_desc = &s_default_depth_stencil,
                                  .blend_desc = &s_default_blend_state,
                              });
#endif

#undef CREATE_PSO

    return Result<void>();
}

void PipelineStateManager::finalize() {
    for (size_t idx = 0; idx < pso_cache_.size(); ++idx) {
        pso_cache_[idx].reset();
    }
}

}  // namespace cave::render
