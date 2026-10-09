#pragma once
#include "EnvironmentFeature.h"

#include "cave/core/diagnostics/Profiler.h"
#include "engine/private/render/render_graph/RenderGraph.h"
#include "engine/private/render/renderer/TransientPool.h"

// @TODO: remove these
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/render/renderer/FrameData.h"

namespace cave::render {

constexpr const char RG_PASS_BAKE_SKYBOX[] = "p:env_skybox";
constexpr const char RG_PASS_BAKE_DIFFUSE[] = "p:diffuse";
constexpr const char RG_PASS_BAKE_PREFILTERED[] = "p:prefiltered";

static void ConvertToCubemapFunc(RenderPassExcutionContext& ctx, int face) {
    CAVE_PROFILE_EVENT();

    auto& cmd = ctx.cmd;

    cmd.setPipelineState(PSO_ENV_SKYBOX_TO_CUBE_MAP);

    cmd.bindConstantBufferSlot<PerBatchConstantBuffer>(cmd.getCurrentFrame().batchCb.get(), face);
    cmd.drawSkybox();
    if (face == 5) {
        GpuTextureId cubemap = ctx.pass.colors[0].tex;
        cmd.generateMipmap(cubemap.get());
    }
}

static void DiffuseIrradianceFunc(RenderPassExcutionContext& ctx, int face) {
    CAVE_PROFILE_EVENT();

    auto& cmd = ctx.cmd;

    cmd.setPipelineState(PSO_DIFFUSE_IRRADIANCE);
    cmd.bindConstantBufferSlot<PerBatchConstantBuffer>(cmd.getCurrentFrame().batchCb.get(), face);
    cmd.drawSkybox();
}

static void PrefilteredFunc(RenderPassExcutionContext& ctx,
                            uint16_t mip,
                            uint16_t face) {
    CAVE_PROFILE_EVENT();

    auto& cmd = ctx.cmd;
    const int index = mip * 6 + face;
    cmd.setPipelineState(PSO_PREFILTER);
    cmd.bindConstantBufferSlot<PerBatchConstantBuffer>(cmd.getCurrentFrame().batchCb.get(), index);
    cmd.drawSkybox();
}

EnvironmentFeature::Outputs EnvironmentFeature::Build(RenderGraph& render_graph, const RenderOptions&) {
    if (m_env_texture) {
        DEV_ASSERT(m_env_cube);
        DEV_ASSERT(m_diffuse);
        DEV_ASSERT(m_specular);

        RGTextureId env_cube_id = render_graph.importTexture({ m_env_cube });
        RGTextureId diffuse_id = render_graph.importTexture({ m_diffuse });
        RGTextureId specular_id = render_graph.importTexture({ m_specular });
        return {
            .skybox = env_cube_id,
            .ibl_diffuse = diffuse_id,
            .ibl_prefiltered = specular_id,
        };
    }

    if (!m_env_texture) {
        const char* path = "sky.hdr";
        // const char* path = "forest.hdr";
        std::shared_ptr<ImageAsset> image = IAssetManager::singleton().findImage(path);
        if (!image) {
            return {};
        }
        m_env_texture = m_device.createTexture(image.get());
    }

    if (!m_env_cube) {
        DEV_ASSERT(!m_diffuse);
        DEV_ASSERT(!m_specular);

        {
            GpuTextureDesc desc = RenderGraph::buildDefaultTextureDesc(
                PixelFormat::R32G32B32A32_FLOAT,
                AttachmentType::COLOR_CUBE,
                RT_SIZE_IBL_CUBEMAP,
                RT_SIZE_IBL_CUBEMAP,
                6,
                RESOURCE_MISC_GENERATE_MIPS,
                kIBLMipChainMax);
            desc.bindFlags |= BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;

            m_env_cube = m_device.createTexture(desc, CubemapSampler());
        }
        {

            GpuTextureDesc desc = RenderGraph::buildDefaultTextureDesc(
                PixelFormat::R32G32B32A32_FLOAT,
                AttachmentType::COLOR_CUBE,
                RT_SIZE_IBL_IRRADIANCE_CUBEMAP,
                RT_SIZE_IBL_IRRADIANCE_CUBEMAP,
                6);
            desc.bindFlags |= BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;

            m_diffuse = m_device.createTexture(desc, CubemapNoMipSampler());
        }
        {
            GpuTextureDesc desc = RenderGraph::buildDefaultTextureDesc(
                PixelFormat::R32G32B32A32_FLOAT,
                AttachmentType::COLOR_CUBE,
                RT_SIZE_IBL_PREFILTERED_CUBEMAP,
                RT_SIZE_IBL_PREFILTERED_CUBEMAP,
                6,
                RESOURCE_MISC_GENERATE_MIPS,
                kIBLMipChainMax);
            desc.bindFlags |= BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;

            m_specular = m_device.createTexture(desc, CubemapLodSampler());
        }
    }

    RGTextureId env_hdr = render_graph.importTexture({ m_env_texture });
    RGTextureId env_cube = render_graph.importTexture({ m_env_cube });
    RGTextureId diffuse = render_graph.importTexture({ m_diffuse });
    RGTextureId specular = render_graph.importTexture({ m_specular });

    // bake environment cubemap
    for (uint16_t face = 0; face < 6; ++face) {
        std::string pass_name = std::format("{}_{}", RG_PASS_BAKE_SKYBOX, face);
        RenderPass& pass = render_graph.addRenderPass(pass_name);

        TextureViewDesc view_desc{};
        view_desc.first_array_slice = face;
        pass.read(ResourceAccess::SRV, env_hdr)
            .writeColor(env_cube, view_desc, LoadOp::Load)
            .setExecuteFunc([face](RenderPassExcutionContext& ctx) {
                ConvertToCubemapFunc(ctx, face);
            });
    }

    // bake irradiance map
    for (uint16_t face = 0; face < 6; ++face) {
        std::string pass_name = std::format("{}_{}", RG_PASS_BAKE_DIFFUSE, face);
        RenderPass& pass = render_graph.addRenderPass(pass_name);

        TextureViewDesc view_desc{};
        view_desc.first_array_slice = face;
        pass.read(ResourceAccess::SRV, env_cube)
            .writeColor(diffuse, view_desc, LoadOp::Load)
            .setExecuteFunc([face](RenderPassExcutionContext& ctx) {
                DiffuseIrradianceFunc(ctx, face);
            });
    }

    int w = RT_SIZE_IBL_PREFILTERED_CUBEMAP;
    int h = RT_SIZE_IBL_PREFILTERED_CUBEMAP;
    for (uint16_t mip = 0; mip < kIBLMipChainMax; ++mip, w /= 2, h /= 2) {
        for (uint16_t face = 0; face < 6; ++face) {
            TextureViewDesc view_desc{
                .mip_slice = mip,
                .first_array_slice = face,
                .array_size = 1,
            };
            std::string pass_name = std::format("{}_{}_{}", RG_PASS_BAKE_PREFILTERED, mip, face);
            RenderPass& pass = render_graph.addRenderPass(pass_name);
            pass.read(ResourceAccess::SRV, env_cube)
                .writeColor(specular, view_desc, LoadOp::Load)
                .setViewport(Viewport(w, h))
                .setExecuteFunc([mip, face](RenderPassExcutionContext& ctx) {
                    PrefilteredFunc(ctx, mip, face);
                });
        }
    }

    return {
        .skybox = env_cube,
        .ibl_diffuse = diffuse,
        .ibl_prefiltered = specular,
    };
}

}  // namespace cave::render
