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

EnvironmentFeature::Outputs EnvironmentFeature::Build(RenderGraphBuilder& p_builder, const FramePlan& p_plan) {
    unused(p_plan);

    RGTextureId env_hdr = p_builder.ImportTexture(
        {
            .debug_name = RG_RES_IBL,
            .func = []() {
                std::shared_ptr<ImageAsset> image = IAssetManager::GetSingleton().FindImage("sky.hdr");
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

    return {
        .ibl_diffuse = env_cube,
        .ibl_prefiltered = env_cube,
    };
}

#if 0
void RenderGraphBuilderExt::AddGenerateSkylightPass() {
    {

        auto& pass = AddPass(RG_PASS_BAKE_SKYBOX);
            .Create(RG_RES_ENV_SKYBOX_CUBE, { desc, CubemapSampler() })
            .Read(ResourceAccess::SRV, RG_RES_IBL)
            .Write(ResourceAccess::RTV, RG_RES_ENV_SKYBOX_CUBE)
            .SetExecuteFunc(ConvertToCubemapFunc);
    }
    {
        GpuTextureDesc desc = BuildDefaultTextureDesc(PixelFormat::R32G32B32A32_FLOAT,
                                                      AttachmentType::COLOR_CUBE,
                                                      RT_SIZE_IBL_IRRADIANCE_CUBEMAP,
                                                      RT_SIZE_IBL_IRRADIANCE_CUBEMAP,
                                                      6);

        auto& pass = AddPass(RG_PASS_BAKE_DIFFUSE);
        pass.Create(RG_RES_ENV_DIFFUSE_CUBE, { desc, CubemapNoMipSampler() })
            .Read(ResourceAccess::SRV, RG_RES_ENV_SKYBOX_CUBE)
            .Write(ResourceAccess::RTV, RG_RES_ENV_DIFFUSE_CUBE)
            .SetExecuteFunc(DiffuseIrradianceFunc);
    }
    {
        GpuTextureDesc desc = BuildDefaultTextureDesc(PixelFormat::R32G32B32A32_FLOAT,
                                                      AttachmentType::COLOR_CUBE,
                                                      RT_SIZE_IBL_PREFILTERED_CUBEMAP,
                                                      RT_SIZE_IBL_PREFILTERED_CUBEMAP,
                                                      6,
                                                      RESOURCE_MISC_GENERATE_MIPS,
                                                      IBL_MIP_CHAIN_MAX);

        auto& pass = AddPass(RG_PASS_BAKE_PREFILTERED);
        pass.Create(RG_RES_ENV_PREFILTERED_CUBE, { desc, CubemapLodSampler() })
            .Read(ResourceAccess::SRV, RG_RES_ENV_SKYBOX_CUBE)
            .Write(ResourceAccess::RTV, RG_RES_ENV_PREFILTERED_CUBE)
            .SetExecuteFunc(PrefilteredFunc);

        AddDependency(RG_PASS_BAKE_PREFILTERED, RG_PASS_DEPTH_PREPASS);
    }
}
#endif

}  // namespace cave::render
