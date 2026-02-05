#pragma once
#include "EnvironmentFeature.h"

#include "cave/core/diagnostics/Profiler.h"
#include "engine/private/render/render_graph/RenderGraph.h"
#include "engine/private/render/renderer/TransientPool.h"

// @TODO: remove these
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/renderer/frame_data.h"

namespace cave::render {

constexpr const char RG_PASS_BAKE_SKYBOX[] = "p:env_skybox";
constexpr const char RG_PASS_BAKE_DIFFUSE[] = "p:diffuse";
constexpr const char RG_PASS_BAKE_PREFILTERED[] = "p:prefiltered";

constexpr const char RG_RES_IBL[] = "r:ibl";
constexpr const char RG_RES_ENV_SKYBOX_CUBE[] = "r:env_cube";
constexpr const char RG_RES_ENV_DIFFUSE_CUBE[] = "r:diffuse_cube";
constexpr const char RG_RES_ENV_PREFILTERED_CUBE[] = "r:prefiltered_cube";

static void ConvertToCubemapFunc(RenderPassExcutionContext& p_ctx, int p_face) {
    CAVE_PROFILE_EVENT();

    auto& cmd = p_ctx.cmd;

    cmd.SetPipelineState(PSO_ENV_SKYBOX_TO_CUBE_MAP);

    cmd.BindConstantBufferSlot<PerBatchConstantBuffer>(cmd.GetCurrentFrame().batchCb.get(), p_face);
    cmd.DrawSkybox();
    if (p_face == 5) {
        GpuTextureId cubemap = p_ctx.pass.colors[0].tex;
        cmd.GenerateMipmap(cubemap.get());
    }
}

static void DiffuseIrradianceFunc(RenderPassExcutionContext& p_ctx, int p_face) {
    CAVE_PROFILE_EVENT();

    auto& cmd = p_ctx.cmd;

    cmd.SetPipelineState(PSO_DIFFUSE_IRRADIANCE);
    cmd.BindConstantBufferSlot<PerBatchConstantBuffer>(cmd.GetCurrentFrame().batchCb.get(), p_face);
    cmd.DrawSkybox();
}

static void PrefilteredFunc(RenderPassExcutionContext& p_ctx, uint16_t p_mip, uint16_t p_face) {
    CAVE_PROFILE_EVENT();

    auto& cmd = p_ctx.cmd;
    const int index = p_mip * 6 + p_face;
    cmd.SetPipelineState(PSO_PREFILTER);
    cmd.BindConstantBufferSlot<PerBatchConstantBuffer>(cmd.GetCurrentFrame().batchCb.get(), index);
    cmd.DrawSkybox();
}

EnvironmentFeature::Outputs EnvironmentFeature::Build(RenderGraph& p_graph, const RenderOptions& p_plan) {
    unused(p_plan);

    if (GpuTextureId env_cube = m_pool.TryGetTexture(RG_RES_ENV_SKYBOX_CUBE)) {
        GpuTextureId diffuse = m_pool.TryGetTexture(RG_RES_ENV_DIFFUSE_CUBE);
        GpuTextureId specular = m_pool.TryGetTexture(RG_RES_ENV_PREFILTERED_CUBE);
        DEV_ASSERT(diffuse);
        DEV_ASSERT(specular);

        RGTextureId env_cube_id = p_graph.ImportTexture({ env_cube });
        RGTextureId diffuse_id = p_graph.ImportTexture({ diffuse });
        RGTextureId specular_id = p_graph.ImportTexture({ specular });
        return {
            .skybox = env_cube_id,
            .ibl_diffuse = diffuse_id,
            .ibl_prefiltered = specular_id,
        };
    }

    if (!m_env_texture) {
        const char* path = "sky.hdr";
        // const char* path = "forest.hdr";
        std::shared_ptr<ImageAsset> image = IAssetManager::GetSingleton().FindImage(path);
        m_env_texture = RenderDevice::GetSingleton().CreateTexture(image.get());
    }

    RGTextureId env_hdr = p_graph.ImportTexture({ m_env_texture });

    RGTextureId env_cube_id = p_graph.CreateTexture({
        .debug_name = RG_RES_ENV_SKYBOX_CUBE,
        .resourceDesc = p_graph.BuildDefaultTextureDesc(
            PixelFormat::R32G32B32A32_FLOAT,
            AttachmentType::COLOR_CUBE,
            RT_SIZE_IBL_CUBEMAP,
            RT_SIZE_IBL_CUBEMAP,
            6,
            RESOURCE_MISC_GENERATE_MIPS,
            IBL_MIP_CHAIN_MAX),
        .samplerDesc = CubemapSampler(),
    });

    RGTextureId ibl_diffuse_id = p_graph.CreateTexture({
        .debug_name = RG_RES_ENV_DIFFUSE_CUBE,
        .resourceDesc = p_graph.BuildDefaultTextureDesc(
            PixelFormat::R32G32B32A32_FLOAT,
            AttachmentType::COLOR_CUBE,
            RT_SIZE_IBL_IRRADIANCE_CUBEMAP,
            RT_SIZE_IBL_IRRADIANCE_CUBEMAP,
            6),
        .samplerDesc = CubemapNoMipSampler(),
    });

    RGTextureId ibl_specular_id = p_graph.CreateTexture({
        .debug_name = RG_RES_ENV_PREFILTERED_CUBE,
        .resourceDesc = p_graph.BuildDefaultTextureDesc(
            PixelFormat::R32G32B32A32_FLOAT,
            AttachmentType::COLOR_CUBE,
            RT_SIZE_IBL_PREFILTERED_CUBEMAP,
            RT_SIZE_IBL_PREFILTERED_CUBEMAP,
            6,
            RESOURCE_MISC_GENERATE_MIPS,
            IBL_MIP_CHAIN_MAX),
        .samplerDesc = CubemapLodSampler(),
    });

    // bake environment cubemap
    for (uint16_t face = 0; face < 6; ++face) {
        std::string pass_name = std::format("{}_{}", RG_PASS_BAKE_SKYBOX, face);
        RenderPass& pass = p_graph.AddPass(pass_name);

        TextureViewDesc view_desc{};
        view_desc.first_array_slice = face;
        pass.Read(ResourceAccess::SRV, env_hdr)
            .WriteColor(env_cube_id, view_desc, LoadOp::Load)
            .SetExecuteFunc([face](RenderPassExcutionContext& p_context) {
                ConvertToCubemapFunc(p_context, face);
            });
    }

    // bake irradiance map
    for (uint16_t face = 0; face < 6; ++face) {
        std::string pass_name = std::format("{}_{}", RG_PASS_BAKE_DIFFUSE, face);
        RenderPass& pass = p_graph.AddPass(pass_name);

        TextureViewDesc view_desc{};
        view_desc.first_array_slice = face;
        pass.Read(ResourceAccess::SRV, env_cube_id)
            .WriteColor(ibl_diffuse_id, view_desc, LoadOp::Load)
            .SetExecuteFunc([face](RenderPassExcutionContext& p_context) {
                DiffuseIrradianceFunc(p_context, face);
            });
    }

    int w = RT_SIZE_IBL_PREFILTERED_CUBEMAP;
    int h = RT_SIZE_IBL_PREFILTERED_CUBEMAP;
    for (uint16_t mip = 0; mip < IBL_MIP_CHAIN_MAX; ++mip, w /= 2, h /= 2) {
        for (uint16_t face = 0; face < 6; ++face) {
            TextureViewDesc view_desc{
                .mip_slice = mip,
                .first_array_slice = face,
                .array_size = 1,
            };
            std::string pass_name = std::format("{}_{}_{}", RG_PASS_BAKE_PREFILTERED, mip, face);
            RenderPass& pass = p_graph.AddPass(pass_name);
            pass.Read(ResourceAccess::SRV, env_cube_id)
                .WriteColor(ibl_specular_id, view_desc, LoadOp::Load)
                .SetViewport(Viewport(w, h))
                .SetExecuteFunc([mip, face](RenderPassExcutionContext& p_context) {
                    PrefilteredFunc(p_context, mip, face);
                });
        }
    }

    m_generated = true;

    return {
        .skybox = env_cube_id,
        .ibl_diffuse = ibl_diffuse_id,
        .ibl_prefiltered = ibl_specular_id,
    };
}

}  // namespace cave::render
