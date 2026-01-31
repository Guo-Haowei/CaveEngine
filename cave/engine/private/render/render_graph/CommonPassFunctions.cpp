#include "CommonPasses.h"

#include "engine/private/algorithm/algorithm.h"
#include "engine/private/assets/image_asset.h"
#include "engine/private/core/base/random.h"
#include "engine/private/core/debugger/Profiler.h"
#include "engine/private/core/math/MatrixTransform.h"
#include "engine/private/renderer/frame_data.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/renderer/graphics_manager.h"
#include "engine/private/renderer/path_tracer_render_system.h"
#include "engine/private/renderer/renderer_misc.h"
#include "engine/private/renderer/sampler.h"
#include "engine/private/runtime/framework/DisplayManager.h"
#include "RenderGraphDefines.h"
#include "RenderPassBuilder.h"

// @TODO: remove
#include "engine/private/renderer/ltc_matrix.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {
#include "shader_resource_defines.hlsl.h"
}  // namespace cave

namespace cave::render {

using namespace cave::math;

// @TODO: generalize this
#if 0
static void DrawInstacedGeometry(const RenderSystem& p_data, const std::vector<InstanceContext>& p_instances, bool p_is_prepass) {
    unused(p_is_prepass);

    CAVE_PROFILE_EVENT();

    auto& gm = IGraphicsManager::GetSingleton();
    auto& frame = gm.GetCurrentFrame();

    for (const auto& instance : p_instances) {
        DEV_ASSERT(instance.instanceBufferIndex >= 0);
        gm.BindConstantBufferSlot<BoneConstantBuffer>(frame.boneCb.get(), instance.instanceBufferIndex);

        gm.BindConstantBufferSlot<PerBatchConstantBuffer>(frame.batchCb.get(), instance.batchIdx);

        gm.SetMesh(instance.gpuMesh);

        const MaterialConstantBuffer& material = p_data.materialCache.buffer[instance.materialIdx];
        gm.BindTexture(Dimension::TEXTURE_2D, material.c_baseColorMapHandle, GetBaseColorMapSlot());
        gm.BindTexture(Dimension::TEXTURE_2D, material.c_normalMapHandle, GetNormalMapSlot());
        gm.BindTexture(Dimension::TEXTURE_2D, material.c_materialMapHandle, GetMaterialMapSlot());

        gm.BindConstantBufferSlot<MaterialConstantBuffer>(frame.materialCb.get(), instance.materialIdx);

        gm.DrawElementsInstanced(instance.instanceCount,
                                 instance.indexCount,
                                 instance.indexOffset);
    }
}
#endif

void ExecuteDrawCommands(RenderPassExcutionContext& p_ctx,
                         const std::vector<DrawItem>& p_commands,
                         bool p_is_prepass) {
    CAVE_PROFILE_EVENT();

    // @TODO: remove
    auto& gm = p_ctx.cmd;
    auto& frame = gm.GetCurrentFrame();
    for (const DrawItem& draw : p_commands) {

        const bool has_bone = draw.bone_idx >= 0;
        if (has_bone) {
            gm.BindConstantBufferSlot<BoneConstantBuffer>(frame.boneCb.get(), draw.bone_idx);
        }

        gm.BindConstantBufferSlot<PerBatchConstantBuffer>(frame.batchCb.get(), draw.batch_idx);

        gm.SetMesh(draw.mesh_data);

        // @TODO: instead of dowing this,
        // set flag directly from draw.flags
        if (p_is_prepass && draw.flags) {
            gm.SetStencilRef(draw.flags);
        }

        if (draw.mat_idx != -1) {
            const MaterialConstantBuffer& material = p_ctx.frameData.materialCache.buffer[draw.mat_idx];
            gm.BindTexture(Dimension::TEXTURE_2D, material.c_baseColorMapHandle, GetBaseColorMapSlot());
            gm.BindTexture(Dimension::TEXTURE_2D, material.c_normalMapHandle, GetNormalMapSlot());
            gm.BindTexture(Dimension::TEXTURE_2D, material.c_materialMapHandle, GetMaterialMapSlot());

            gm.BindConstantBufferSlot<MaterialConstantBuffer>(frame.materialCb.get(), draw.mat_idx);
        }
        gm.DrawElements(draw.index.count, draw.index.offset);

        if (p_is_prepass && draw.flags) {
            gm.SetStencilRef(0);
        }
    }
}

struct ScopedEvent {
    IRenderCmdContext& m_ctx;

    ScopedEvent(IRenderCmdContext& p_ctx, std::string_view p_name)
        : m_ctx(p_ctx) {
        m_ctx.BeginEvent(p_name);
    }

    ~ScopedEvent() {
        m_ctx.EndEvent();
    }
};

#define RENDER_PASS_FUNC()                                \
    ScopedEvent _scoped(p_ctx.cmd, p_ctx.pass.GetName()); \
    CAVE_PROFILE_EVENT();

void DepthPrepassFunc(RenderPassExcutionContext& p_ctx) {
    RENDER_PASS_FUNC();

    Framebuffer* fb = p_ctx.framebuffer;
    auto& cmd = p_ctx.cmd;
    auto& frame = cmd.GetCurrentFrame();
    const uint32_t width = fb->desc.depthAttachment->desc.width;
    const uint32_t height = fb->desc.depthAttachment->desc.height;

    cmd.SetRenderTarget(fb);
    cmd.SetViewport(Viewport(width, height));

    const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    cmd.Clear(fb, CLEAR_DEPTH_BIT | CLEAR_STENCIL_BIT, clear_color, 0.0f, STENCIL_FLAG_SKY);

    const auto& prepass_commands = p_ctx.frameData.commands[std::to_underlying(DrawPhase::DepthPrepass)];
    if (prepass_commands.empty()) {
        return;
    }

    const PassContext& pass = p_ctx.frameData.mainPass;
    cmd.BindConstantBufferSlot<PerPassConstantBuffer>(frame.passCb.get(), pass.pass_idx);

    cmd.SetPipelineState(PSO_PREPASS);
    ExecuteDrawCommands(p_ctx, prepass_commands, true);
}

void GbufferPassFunc(RenderPassExcutionContext& p_ctx) {
    RENDER_PASS_FUNC();

    const Framebuffer* fb = p_ctx.framebuffer;
    auto& cmd = p_ctx.cmd;

    const auto& frame = cmd.GetCurrentFrame();
    const uint32_t width = fb->desc.depthAttachment->desc.width;
    const uint32_t height = fb->desc.depthAttachment->desc.height;

    cmd.SetRenderTarget(fb);
    cmd.SetViewport(Viewport(width, height));

#if 0
    const float clear_color[4] = { .3f, .3f, .3f, 1.0f };
#else
    const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
#endif
    cmd.Clear(fb, CLEAR_COLOR_BIT, clear_color);

    const auto& deferred_commands = p_ctx.frameData.commands[std::to_underlying(DrawPhase::Deferred)];
    if (deferred_commands.empty()) {
        return;
    }

    const PassContext& pass = p_ctx.frameData.mainPass;
    cmd.BindConstantBufferSlot<PerPassConstantBuffer>(frame.passCb.get(), pass.pass_idx);

    cmd.SetPipelineState(PSO_GBUFFER);
    ExecuteDrawCommands(p_ctx, deferred_commands, false);
    // DrawInstacedGeometry(p_ctx.render_system, p_ctx.render_system.instances, false);
    cmd.SetPipelineState(PSO_GBUFFER_DOUBLE_SIDED);
}

// textures generated by program

void SsaoPassFunc(RenderPassExcutionContext& p_ctx) {
    if (!p_ctx.frameData.options.ssaoEnabled) {
        return;
    }

    RENDER_PASS_FUNC();

    auto& cmd = p_ctx.cmd;

    auto fb = p_ctx.framebuffer;
    const uint32_t width = fb->desc.colorAttachments[0]->desc.width;
    const uint32_t height = fb->desc.colorAttachments[0]->desc.height;

    cmd.SetRenderTarget(fb);
    cmd.SetViewport(Viewport(width, height));
    cmd.Clear(fb, CLEAR_COLOR_BIT);

    cmd.SetPipelineState(PSO_SSAO);
    cmd.SetMesh(nullptr);
    cmd.DrawArrays(6);
}

void HighlightPassFunc(RenderPassExcutionContext& p_ctx) {
    RENDER_PASS_FUNC();

    auto fb = p_ctx.framebuffer;
    auto& cmd = p_ctx.cmd;
    cmd.SetRenderTarget(fb);
    const auto [width, height] = fb->GetBufferSize();

    cmd.SetViewport(Viewport(width, height));

    cmd.SetPipelineState(PSO_HIGHLIGHT);
    cmd.SetStencilRef(STENCIL_FLAG_SELECTED);
    cmd.Clear(fb, CLEAR_COLOR_BIT);
    cmd.SetMesh(nullptr);
    cmd.DrawArrays(6);
    cmd.SetStencilRef(0);
}

void VoxelizationPassFunc(RenderPassExcutionContext& p_ctx) {
    if (p_ctx.frameData.voxelPass.pass_idx < 0) {
        return;
    }

    RENDER_PASS_FUNC();
    auto& cmd = p_ctx.cmd;
    const auto& frame = cmd.GetCurrentFrame();

    const int voxel_size = DVAR_GET_INT(gfx_voxel_size);

    // post process
    const uint32_t group_size = voxel_size / COMPUTE_LOCAL_SIZE_VOXEL;
    cmd.SetPipelineState(PSO_VOXELIZATION_PRE);
    cmd.Dispatch(group_size, group_size, group_size);

    const PassContext& pass = p_ctx.frameData.voxelPass;
    cmd.BindConstantBufferSlot<PerPassConstantBuffer>(frame.passCb.get(), pass.pass_idx);

    // @TODO: hack
    if (cmd.GetBackend() == Backend::OPENGL) {
        cmd.SetViewport(Viewport(voxel_size, voxel_size));
        cmd.SetPipelineState(PSO_VOXELIZATION);
        cmd.SetBlendState(PipelineStateManager::GetBlendDescDisable(), nullptr, 0xFFFFFFFF);
        ExecuteDrawCommands(p_ctx, p_ctx.frameData.commands[std::to_underlying(DrawPhase::Voxelization)], false);

        // glSubpixelPrecisionBiasNV(0, 0);
        cmd.SetBlendState(PipelineStateManager::GetBlendDescDefault(), nullptr, 0xFFFFFFFF);
    }

    // post process
    cmd.SetPipelineState(PSO_VOXELIZATION_POST);
    cmd.Dispatch(group_size, group_size, group_size);

    for (auto& uav : p_ctx.pass.GetUavs()) {
        cmd.GenerateMipmap(uav.get());
    }

    // @TODO: [SCRUM-28] refactor
    cmd.UnsetRenderTarget();
}

/// Emitter
static void EmitterPassFunc(RenderPassExcutionContext& p_ctx) {
    unused(p_ctx);
#if 0
    RENDER_PASS_FUNC();

    auto& cmd = p_ctx.cmd;
    auto fb = p_ctx.framebuffer;
    auto& render_system = p_ctx.render_system;
    auto& frame = cmd.GetCurrentFrame();
    const auto [width, height] = fb->GetBufferSize();

    cmd.SetRenderTarget(fb);
    cmd.SetViewport(Viewport(width, height));

    const PassContext& pass = render_system.mainPass;
    cmd.BindConstantBufferSlot<PerPassConstantBuffer>(frame.passCb.get(), pass.pass_idx);

    int particle_idx = 0;
    for (const auto& emitter : render_system.emitters) {
        if (!emitter.particleBuffer) {
            continue;
        }

        cmd.BindConstantBufferSlot<EmitterConstantBuffer>(frame.emitterCb.get(), particle_idx);
        ++particle_idx;

        cmd.BindStructuredBuffer(GetGlobalParticleCounterSlot(), emitter.counterBuffer.get());
        cmd.BindStructuredBuffer(GetGlobalDeadIndicesSlot(), emitter.deadBuffer.get());
        cmd.BindStructuredBuffer(GetGlobalAliveIndicesPreSimSlot(), emitter.aliveBuffer[emitter.GetPreIndex()].get());
        cmd.BindStructuredBuffer(GetGlobalAliveIndicesPostSimSlot(), emitter.aliveBuffer[emitter.GetPostIndex()].get());
        cmd.BindStructuredBuffer(GetGlobalParticleDataSlot(), emitter.particleBuffer.get());

        cmd.SetPipelineState(PSO_PARTICLE_KICKOFF);
        cmd.Dispatch(1, 1, 1);

        cmd.SetPipelineState(PSO_PARTICLE_EMIT);
        cmd.Dispatch(MAX_PARTICLE_COUNT / PARTICLE_LOCAL_SIZE, 1, 1);

        cmd.SetPipelineState(PSO_PARTICLE_SIM);
        cmd.Dispatch(MAX_PARTICLE_COUNT / PARTICLE_LOCAL_SIZE, 1, 1);

        cmd.UnbindStructuredBuffer(GetGlobalParticleCounterSlot());
        cmd.UnbindStructuredBuffer(GetGlobalDeadIndicesSlot());
        cmd.UnbindStructuredBuffer(GetGlobalAliveIndicesPreSimSlot());
        cmd.UnbindStructuredBuffer(GetGlobalAliveIndicesPostSimSlot());
        cmd.UnbindStructuredBuffer(GetGlobalParticleDataSlot());

        // Renderering
        cmd.SetPipelineState(PSO_PARTICLE_RENDERING);

        bool use_texture = false;
        if (!emitter.texture.empty()) {
            const ImageAsset* image = AssetRegistry::GetSingleton().Request<ImageAsset>(emitter.texture);
            if (image && image->gpu_texture) {
                cmd.BindTexture(Dimension::TEXTURE_2D, image->gpu_texture->GetHandle(), GetBaseColorMapSlot());
                use_texture = true;
            }
        }

        cmd.BindStructuredBufferSRV(GetGlobalParticleDataSlot(), emitter.particleBuffer.get());
        cmd.DrawQuadInstanced(MAX_PARTICLE_COUNT);
        cmd.UnbindStructuredBufferSRV(GetGlobalParticleDataSlot());

        if (use_texture) {
            cmd.UnbindTexture(Dimension::TEXTURE_2D, GetBaseColorMapSlot());
        }
    }
#endif
}

/// Lighting
void LightingPassFunc(RenderPassExcutionContext& p_ctx) {
    RENDER_PASS_FUNC();

    auto& cmd = p_ctx.cmd;
    auto fb = p_ctx.framebuffer;
    const auto [width, height] = fb->GetBufferSize();

    cmd.SetRenderTarget(fb);

    cmd.SetViewport(Viewport(width, height));
    const float clear_color[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    cmd.Clear(fb, CLEAR_COLOR_BIT);
    cmd.SetPipelineState(PSO_LIGHTING);

    cmd.SetMesh(nullptr);
    cmd.DrawArrays(6);
}

/// Sky
void ForwardPassFunc(RenderPassExcutionContext& p_ctx) {
    RENDER_PASS_FUNC();

    auto& gm = p_ctx.cmd;

    auto fb = p_ctx.framebuffer;
    const auto [width, height] = fb->GetBufferSize();

    gm.SetRenderTarget(fb);

    gm.SetViewport(Viewport(width, height));

    const PassContext& pass = p_ctx.frameData.mainPass;
    gm.BindConstantBufferSlot<PerPassConstantBuffer>(gm.GetCurrentFrame().passCb.get(), pass.pass_idx);

    // if (p_ctx.frameData.options.iblEnabled)
    {
        gm.SetPipelineState(PSO_ENV_SKYBOX);
        gm.SetStencilRef(STENCIL_FLAG_SKY);
        gm.DrawSkybox();
        gm.SetStencilRef(0);
    }

    // draw transparent objects
    gm.SetPipelineState(PSO_FORWARD_TRANSPARENT);
    ExecuteDrawCommands(p_ctx, p_ctx.frameData.commands[std::to_underlying(DrawPhase::Forward)], false);

    EmitterPassFunc(p_ctx);
}

/// Bloom
void BloomSetupFunc(RenderPassExcutionContext& p_ctx) {
    if (!p_ctx.frameData.options.bloomEnabled) {
        return;
    }

    RENDER_PASS_FUNC();

    auto& cmd = p_ctx.cmd;

    auto uav = p_ctx.pass.GetUavs()[0];

    cmd.SetPipelineState(PSO_BLOOM_SETUP);

    const uint32_t work_group_x = CeilingDivision(uav->desc.width, 16);
    const uint32_t work_group_y = CeilingDivision(uav->desc.height, 16);

    cmd.Dispatch(work_group_x, work_group_y, 1);
}

void BloomDownSampleFunc(RenderPassExcutionContext& p_ctx) {
    if (!p_ctx.frameData.options.bloomEnabled) {
        return;
    }

    RENDER_PASS_FUNC();

    auto& cmd = p_ctx.cmd;

    cmd.SetPipelineState(PSO_BLOOM_DOWNSAMPLE);

    auto uav = p_ctx.pass.GetUavs()[0];

    const uint32_t work_group_x = CeilingDivision(uav->desc.width, 16);
    const uint32_t work_group_y = CeilingDivision(uav->desc.height, 16);

    cmd.Dispatch(work_group_x, work_group_y, 1);
}

void BloomUpSampleFunc(RenderPassExcutionContext& p_ctx) {
    if (!p_ctx.frameData.options.bloomEnabled) {
        return;
    }

    RENDER_PASS_FUNC();

    auto& cmd = p_ctx.cmd;

    auto uav = p_ctx.pass.GetUavs()[0];

    cmd.SetPipelineState(PSO_BLOOM_UPSAMPLE);

    const uint32_t work_group_x = CeilingDivision(uav->desc.width, 16);
    const uint32_t work_group_y = CeilingDivision(uav->desc.height, 16);

    cmd.Dispatch(work_group_x, work_group_y, 1);
}

// @TODO: get rid off this!
void DebugVoxels(RenderPassExcutionContext& p_ctx) {
    CAVE_PROFILE_EVENT();

    auto& gm = p_ctx.cmd;
    auto p_framebuffer = p_ctx.framebuffer;
    gm.SetRenderTarget(p_framebuffer);
    auto depth_buffer = p_framebuffer->desc.depthAttachment;
    const auto [width, height] = p_framebuffer->GetBufferSize();

    // glEnable(GL_BLEND);
    gm.SetViewport(Viewport(width, height));
    gm.Clear(p_framebuffer, CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, IGraphicsManager::DEFAULT_CLEAR_COLOR, 0.0f);

    p_ctx.cmd.SetPipelineState(PSO_DEBUG_VOXEL);

    gm.SetMesh(gm.m_boxBuffers.get());
    const uint32_t size = DVAR_GET_INT(gfx_voxel_size);
    gm.DrawElementsInstanced(size * size * size, gm.m_boxBuffers->desc.drawCount);

    // glDisable(GL_BLEND);
}

/// Tone
/// Change to post processing?
void TonePassFunc(RenderPassExcutionContext& p_ctx) {
    RENDER_PASS_FUNC();

    auto& cmd = p_ctx.cmd;
    auto fb = p_ctx.framebuffer;
    cmd.SetRenderTarget(fb);

    auto depth_buffer = fb->desc.depthAttachment;
    const auto [width, height] = fb->GetBufferSize();

    // draw billboards

    cmd.SetViewport(Viewport(width, height));
    cmd.Clear(fb, CLEAR_COLOR_BIT);

    cmd.SetPipelineState(PSO_POST_PROCESS);
    cmd.SetMesh(nullptr);
    cmd.DrawArrays(6);
}

void ConvertToCubemapFunc(RenderPassExcutionContext& p_ctx) {
    if (!p_ctx.frameData.bakeIbl) {
        return;
    }

    RENDER_PASS_FUNC();

    auto& cmd = p_ctx.cmd;
    auto fb = p_ctx.framebuffer;

    cmd.SetPipelineState(PSO_ENV_SKYBOX_TO_CUBE_MAP);
    auto cube_map = fb->desc.colorAttachments[0];
    const auto [width, height] = fb->GetBufferSize();

    auto& frame = cmd.GetCurrentFrame();
    for (int i = 0; i < 6; ++i) {
        cmd.SetRenderTarget(fb, i);

        cmd.SetViewport(Viewport(width, height));

        cmd.BindConstantBufferSlot<PerBatchConstantBuffer>(frame.batchCb.get(), i);
        cmd.DrawSkybox();
    }
    cmd.GenerateMipmap(cube_map.get());
}

void DiffuseIrradianceFunc(RenderPassExcutionContext& p_ctx) {
    if (!p_ctx.frameData.bakeIbl) {
        return;
    }

    RENDER_PASS_FUNC();

    auto& cmd = p_ctx.cmd;
    auto fb = p_ctx.framebuffer;

    cmd.SetPipelineState(PSO_DIFFUSE_IRRADIANCE);
    const auto [width, height] = fb->GetBufferSize();

    auto& frame = cmd.GetCurrentFrame();
    for (int i = 0; i < 6; ++i) {
        cmd.SetRenderTarget(fb, i);
        cmd.SetViewport(Viewport(width, height));

        cmd.BindConstantBufferSlot<PerBatchConstantBuffer>(frame.batchCb.get(), i);
        cmd.DrawSkybox();
    }
}

void PrefilteredFunc(RenderPassExcutionContext& p_ctx) {
    if (!p_ctx.frameData.bakeIbl) {
        return;
    }

    RENDER_PASS_FUNC();

    auto& cmd = p_ctx.cmd;
    auto fb = p_ctx.framebuffer;

    cmd.SetPipelineState(PSO_PREFILTER);
    auto [width, height] = fb->GetBufferSize();

    auto& frame = cmd.GetCurrentFrame();
    for (int mip_idx = 0; mip_idx < IBL_MIP_CHAIN_MAX; ++mip_idx, width /= 2, height /= 2) {
        for (int face_id = 0; face_id < 6; ++face_id) {
            const int index = mip_idx * 6 + face_id;
            cmd.BindConstantBufferSlot<PerBatchConstantBuffer>(frame.batchCb.get(), index);

            cmd.SetRenderTarget(fb, face_id, mip_idx);
            cmd.SetViewport(Viewport(width, height));
            cmd.DrawSkybox();
        }
    }
}

void PathTracerPassFunc(RenderPassExcutionContext& p_ctx) {
    // @TODO: refactor this part
    if (!IsPathTracerActive()) {
        return;
    }

    auto& cmd = p_ctx.cmd;

    cmd.SetPipelineState(PSO_PATH_TRACER);
    const auto& input = p_ctx.pass.GetUavs()[0];

    DEV_ASSERT(input);

    const uint32_t width = input->desc.width;
    const uint32_t height = input->desc.height;
    const uint32_t work_group_x = CeilingDivision(width, 16);
    const uint32_t work_group_y = CeilingDivision(height, 16);

    // @TODO: transition
    BindPathTracerData(cmd);
    cmd.Dispatch(work_group_x, work_group_y, 1);
    UnbindPathTracerData(cmd);
}

void PathTracerTonePassFunc(RenderPassExcutionContext& p_ctx) {
    RENDER_PASS_FUNC();

    auto& cmd = p_ctx.cmd;
    auto fb = p_ctx.framebuffer;
    cmd.SetRenderTarget(fb);

    auto depth_buffer = fb->desc.depthAttachment;
    const auto [width, height] = fb->GetBufferSize();

    cmd.SetViewport(Viewport(width, height));
    cmd.Clear(fb, CLEAR_COLOR_BIT);

    cmd.SetPipelineState(PSO_POST_PROCESS);
    cmd.SetMesh(nullptr);
    cmd.DrawArrays(6);
}

}  // namespace cave::render
