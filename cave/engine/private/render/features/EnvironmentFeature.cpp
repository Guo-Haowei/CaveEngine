#pragma once
#include "EnvironmentFeature.h"

#include "engine/private/core/debugger/Profiler.h"
#include "engine/private/render/render_graph/RenderGraphBuilder.h"

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

    if (!p_ctx.frameData.bakeIbl) {
        return;
    }

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

    if (!p_ctx.frameData.bakeIbl) {
        return;
    }

    auto& cmd = p_ctx.cmd;

    cmd.SetPipelineState(PSO_DIFFUSE_IRRADIANCE);
    cmd.BindConstantBufferSlot<PerBatchConstantBuffer>(cmd.GetCurrentFrame().batchCb.get(), p_face);
    cmd.DrawSkybox();
}

static void PrefilteredFunc(RenderPassExcutionContext& p_ctx, uint16_t p_mip, uint16_t p_face) {
    CAVE_PROFILE_EVENT();
    if (!p_ctx.frameData.bakeIbl) {
        return;
    }

    auto& cmd = p_ctx.cmd;
    const int index = p_mip * 6 + p_face;
    cmd.SetPipelineState(PSO_PREFILTER);
    cmd.BindConstantBufferSlot<PerBatchConstantBuffer>(cmd.GetCurrentFrame().batchCb.get(), index);
    cmd.DrawSkybox();
}

EnvironmentFeature::Outputs EnvironmentFeature::Build(RenderGraphBuilder& p_builder, const FramePlan& p_plan) {
    unused(p_plan);

    RGTextureId env_hdr = p_builder.ImportTexture(
        {
            .debug_name = RG_RES_IBL,
            .func = []() {
                const char* path = "sky.hdr";
                // const char* path = "forest.hdr";
                std::shared_ptr<ImageAsset> image = IAssetManager::GetSingleton().FindImage(path);
                return RenderDevice::GetSingleton().CreateTexture(image.get());
            },
        });

    RGTextureId env_cube = p_builder.CreateTexture({
        .debug_name = RG_RES_ENV_SKYBOX_CUBE,
        .resourceDesc = p_builder.BuildDefaultTextureDesc(
            PixelFormat::R32G32B32A32_FLOAT,
            AttachmentType::COLOR_CUBE,
            RT_SIZE_IBL_CUBEMAP,
            RT_SIZE_IBL_CUBEMAP,
            6,
            RESOURCE_MISC_GENERATE_MIPS,
            IBL_MIP_CHAIN_MAX),
        .samplerDesc = CubemapSampler(),
    });

    RGTextureId ibl_diffuse = p_builder.CreateTexture({
        .debug_name = RG_RES_ENV_DIFFUSE_CUBE,
        .resourceDesc = p_builder.BuildDefaultTextureDesc(
            PixelFormat::R32G32B32A32_FLOAT,
            AttachmentType::COLOR_CUBE,
            RT_SIZE_IBL_IRRADIANCE_CUBEMAP,
            RT_SIZE_IBL_IRRADIANCE_CUBEMAP,
            6),
        .samplerDesc = CubemapNoMipSampler(),
    });

    RGTextureId ibl_prefiltered = p_builder.CreateTexture({
        .debug_name = RG_RES_ENV_PREFILTERED_CUBE,
        .resourceDesc = p_builder.BuildDefaultTextureDesc(
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
        RenderPassBuilder& pass = p_builder.AddPass(pass_name);

        TextureViewDesc view_desc{};
        view_desc.first_array_slice = face;
        pass.Read(ResourceAccess::SRV, env_hdr)
            .WriteColor(env_cube, view_desc, LoadOp::Load)
            .SetExecuteFunc([face](RenderPassExcutionContext& p_context) {
                ConvertToCubemapFunc(p_context, face);
            });
    }

    // bake irradiance map
    for (uint16_t face = 0; face < 6; ++face) {
        std::string pass_name = std::format("{}_{}", RG_PASS_BAKE_DIFFUSE, face);
        RenderPassBuilder& pass = p_builder.AddPass(pass_name);

        TextureViewDesc view_desc{};
        view_desc.first_array_slice = face;
        pass.Read(ResourceAccess::SRV, env_cube)
            .WriteColor(ibl_diffuse, view_desc, LoadOp::Load)
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
            RenderPassBuilder& pass = p_builder.AddPass(pass_name);
            pass.Read(ResourceAccess::SRV, env_cube)
                .WriteColor(ibl_prefiltered, view_desc, LoadOp::Load)
                .SetViewport(Viewport(w, h))
                .SetExecuteFunc([mip, face](RenderPassExcutionContext& p_context) {
                    PrefilteredFunc(p_context, mip, face);
                });
        }
    }

    return {
        .skybox = env_cube,
        .ibl_diffuse = ibl_diffuse,
        .ibl_prefiltered = ibl_prefiltered,
    };
}

}  // namespace cave::render
