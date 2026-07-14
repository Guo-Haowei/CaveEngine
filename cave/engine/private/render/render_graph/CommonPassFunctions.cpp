#include "CommonPasses.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/display/DisplayService.h"
#include "cave/runtime/display/ICanvas.h"

#include "engine/private/algorithm/algorithm.h"
#include "engine/private/render/renderer/FrameData.h"
#include "engine/private/render/renderer/Renderer.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/renderer/renderer_misc.h"
#include "engine/private/renderer/sampler.h"
#include "RenderGraphDefines.h"
#include "RenderPass.h"

// @TODO: remove
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

    auto& gm = IRenderDevice::singleton();
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
        const bool has_flags = p_is_prepass && draw.flags;
        if (has_flags) {
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

        if (has_flags) {
            gm.SetStencilRef(STENCIL_FLAG_NONE);
        }
    }
}

void DepthPrepassFunc(RenderPassExcutionContext& p_ctx) {
    CAVE_PROFILE_EVENT();

    auto& cmd = p_ctx.cmd;
    auto& frame = cmd.GetCurrentFrame();

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
    CAVE_PROFILE_EVENT();

    auto& cmd = p_ctx.cmd;
    const auto& frame = cmd.GetCurrentFrame();

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

void HighlightPassFunc(RenderPassExcutionContext& p_ctx) {
    CAVE_PROFILE_EVENT();

    auto& cmd = p_ctx.cmd;

    cmd.SetPipelineState(PSO_HIGHLIGHT);
    cmd.SetStencilRef(STENCIL_FLAG_HIGHLIGHT);
    cmd.SetMesh(nullptr);
    cmd.DrawArrays(6);
    cmd.SetStencilRef(STENCIL_FLAG_NONE);
}

#if 0
void VoxelizationPassFunc(RenderPassExcutionContext& p_ctx) {
    CAVE_PROFILE_EVENT();

    DEV_ASSERT(0);
    if (p_ctx.frameData.voxelPass.pass_idx < 0) {
        return;
    }

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
        cmd.SetBlendState(PipelineStateManager::blendDescDisabled(), nullptr, 0xFFFFFFFF);
        ExecuteDrawCommands(p_ctx, p_ctx.frameData.commands[std::to_underlying(DrawPhase::Voxelization)], false);

        // glSubpixelPrecisionBiasNV(0, 0);
        cmd.SetBlendState(PipelineStateManager::defaultBlendDesc(), nullptr, 0xFFFFFFFF);
    }

    // post process
    cmd.SetPipelineState(PSO_VOXELIZATION_POST);
    cmd.Dispatch(group_size, group_size, group_size);

    for (auto& uav : p_ctx.pass.uavs) {
        cmd.GenerateMipmap(uav.get());
    }

    // @TODO: [SCRUM-28] refactor
    cmd.UnsetRenderTargets();
}

/// Emitter
static void EmitterPassFunc(RenderPassExcutionContext& p_ctx) {
    unused(p_ctx);
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
            const ImageAsset* image = AssetRegistry::singleton().Request<ImageAsset>(emitter.texture);
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
}
#endif

/// Lighting
void LightingPassFunc(RenderPassExcutionContext& p_ctx) {
    CAVE_PROFILE_EVENT();

    auto& cmd = p_ctx.cmd;
    cmd.SetPipelineState(PSO_LIGHTING);

    cmd.SetMesh(nullptr);
    cmd.DrawArrays(6);
}

/// Sky
void ForwardPassFunc(RenderPassExcutionContext& p_ctx) {
    CAVE_PROFILE_EVENT();

    auto& gm = p_ctx.cmd;

    const PassContext& pass = p_ctx.frameData.mainPass;
    gm.BindConstantBufferSlot<PerPassConstantBuffer>(gm.GetCurrentFrame().passCb.get(), pass.pass_idx);

    gm.SetPipelineState(PSO_ENV_SKYBOX);
    gm.SetStencilRef(STENCIL_FLAG_SKY);
    gm.DrawSkybox();
    gm.SetStencilRef(STENCIL_FLAG_NONE);

    // draw transparent objects
    gm.SetPipelineState(PSO_FORWARD_TRANSPARENT);
    ExecuteDrawCommands(p_ctx, p_ctx.frameData.commands[std::to_underlying(DrawPhase::Forward)], false);
}

/// Bloom
void BloomSetupFunc(RenderPassExcutionContext& p_ctx) {
    CAVE_PROFILE_EVENT();

    if (!p_ctx.frameData.options.enable_bloom) {
        return;
    }

    auto& cmd = p_ctx.cmd;

    auto uav = p_ctx.pass.uavs[0];

    cmd.SetPipelineState(PSO_BLOOM_SETUP);

    const uint32_t work_group_x = CeilingDivision(uav->desc.width, 16);
    const uint32_t work_group_y = CeilingDivision(uav->desc.height, 16);

    cmd.Dispatch(work_group_x, work_group_y, 1);
}

void BloomDownSampleFunc(RenderPassExcutionContext& p_ctx) {
    CAVE_PROFILE_EVENT();

    if (!p_ctx.frameData.options.enable_bloom) {
        return;
    }

    auto& cmd = p_ctx.cmd;

    cmd.SetPipelineState(PSO_BLOOM_DOWNSAMPLE);

    auto uav = p_ctx.pass.uavs[0];

    const uint32_t work_group_x = CeilingDivision(uav->desc.width, 16);
    const uint32_t work_group_y = CeilingDivision(uav->desc.height, 16);

    cmd.Dispatch(work_group_x, work_group_y, 1);
}

void BloomUpSampleFunc(RenderPassExcutionContext& p_ctx) {
    CAVE_PROFILE_EVENT();

    if (!p_ctx.frameData.options.enable_bloom) {
        return;
    }

    auto& cmd = p_ctx.cmd;

    auto uav = p_ctx.pass.uavs[0];

    cmd.SetPipelineState(PSO_BLOOM_UPSAMPLE);

    const uint32_t work_group_x = CeilingDivision(uav->desc.width, 16);
    const uint32_t work_group_y = CeilingDivision(uav->desc.height, 16);

    cmd.Dispatch(work_group_x, work_group_y, 1);
}

/// Tone
/// Change to post processing?
void TonePassFunc(RenderPassExcutionContext& ctx) {
    CAVE_PROFILE_EVENT();

    auto& cmd = ctx.cmd;

    cmd.SetPipelineState(PSO_POST_PROCESS);
    cmd.SetMesh(nullptr);
    cmd.DrawArrays(6);

    if (auto ui_renderer = ctx.services.renderer().tryGet<UIRenderer>()) {
        ui_renderer->drawCanvas(ctx.cmd,
                                ctx.services.UICanvas(),
                                ctx.frameData.view_id);
    }
}

void Pass2DDrawFunc(RenderPassExcutionContext& ctx) {
    CAVE_PROFILE_EVENT();

    auto& cmd = ctx.cmd;
    auto& frame = cmd.GetCurrentFrame();
    const PassContext& pass = ctx.frameData.mainPass;

    cmd.BindConstantBufferSlot<PerPassConstantBuffer>(frame.passCb.get(), pass.pass_idx);

    if (!ctx.frameData.tile_maps.empty()) {
        cmd.SetPipelineState(PSO_SPRITE);
        for (const DrawItem& draw : ctx.frameData.tile_maps) {
            const auto tile = draw.mesh_data;
            if (draw.texture) {
                cmd.BindTexture(Dimension::TEXTURE_2D, draw.texture->GetHandle(), 0);
            }
            cmd.SetMesh(tile);
            cmd.BindConstantBufferSlot<PerBatchConstantBuffer>(frame.batchCb.get(), draw.batch_idx);
            cmd.DrawElementsInstanced(1, draw.index.count);
        }
    }

    if (!ctx.frameData.sprites.empty()) {
        cmd.SetMesh(nullptr);
        cmd.SetPipelineState(PSO_SPRITE_NO_VERT);
        for (const DrawItem& draw : ctx.frameData.sprites) {
            DEV_ASSERT(draw.mesh_data == nullptr);
            if (draw.texture) {
                cmd.BindTexture(Dimension::TEXTURE_2D, draw.texture->GetHandle(), 0);
            }
            cmd.BindConstantBufferSlot<PerBatchConstantBuffer>(frame.batchCb.get(), draw.batch_idx);
            cmd.DrawArrays(draw.index.count);
        }
    }

    // @TODO: move this to a different pass
    if (auto overlay = ctx.services.renderer().tryGet<OverlayRenderer>()) {
        overlay->drawCanvas(cmd,
                            ctx.services.canvas(),
                            ctx.frameData.view_id);
    }
}

}  // namespace cave::render
