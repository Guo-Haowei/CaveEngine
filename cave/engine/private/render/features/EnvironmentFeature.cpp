#include "EnvironmentFeature.h"

#include "cave/core/diagnostics/Profiler.h"
#include "engine/private/render/render_graph/RenderGraph.h"
#include "engine/private/render/renderer/TransientPool.h"

// @TODO: remove these
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/IAssetManager.h"
#include "engine/private/render/rhi/RenderDevice.h"
#include "engine/private/render/renderer/FrameData.h"

namespace cave::render {

namespace {

constexpr const char kPassBakeSkybox[] = "p:env_skybox";
constexpr const char kPassBakeDiffuse[] = "p:diffuse";
constexpr const char kPassBakePrefiltered[] = "p:prefiltered";

constexpr int kCubemapSize = 512;
constexpr int kIrradianceCubemapSize = 32;
constexpr int kPrefilteredCubemapSize = 512;

void ConvertToCubemapFunc(RenderPassExcutionContext& ctx, int face) {
    CAVE_PROFILE_EVENT();

    auto& device = ctx.device;
    auto& cmd = ctx.command;

    cmd.setPipelineState(PipelineStateName::EnvSkyboxToCubemap);

    cmd.bindConstantBufferSlot<PerBatchConstantBuffer>(device.getCurrentFrame().batch_cb.get(), face);
    device.drawSkybox();
    if (face == 5) {
        GpuTextureId cubemap = ctx.pass.colors[0].tex;
        device.generateMipmap(cubemap.get());
    }
}

void DiffuseIrradianceFunc(RenderPassExcutionContext& ctx, int face) {
    CAVE_PROFILE_EVENT();

    auto& device = ctx.device;
    auto& cmd = ctx.command;

    cmd.setPipelineState(PipelineStateName::DiffuseIrradiance);
    cmd.bindConstantBufferSlot<PerBatchConstantBuffer>(device.getCurrentFrame().batch_cb.get(), face);
    device.drawSkybox();
}

void PrefilteredFunc(RenderPassExcutionContext& ctx,
                     uint16_t mip,
                     uint16_t face) {
    CAVE_PROFILE_EVENT();

    auto& device = ctx.device;
    auto& cmd = ctx.command;

    const int index = mip * 6 + face;
    cmd.setPipelineState(PipelineStateName::Prefilter);
    cmd.bindConstantBufferSlot<PerBatchConstantBuffer>(device.getCurrentFrame().batch_cb.get(), index);
    device.drawSkybox();
}

}  // namespace

EnvironmentFeature::Outputs EnvironmentFeature::build(RenderGraph& graph, const RenderOptions&) {
    if (m_env_texture) {
        DEV_ASSERT(m_env_cube);
        DEV_ASSERT(m_diffuse);
        DEV_ASSERT(m_specular);

        RGTextureId env_cube_id = graph.importResource({ m_env_cube });
        RGTextureId diffuse_id = graph.importResource({ m_diffuse });
        RGTextureId specular_id = graph.importResource({ m_specular });
        return {
            .skybox = env_cube_id,
            .ibl_diffuse = diffuse_id,
            .ibl_prefiltered = specular_id,
        };
    }

    if (!m_env_texture) {
#if 1
        const char* path = "sky.hdr";
#else
        const char* path = "test.hdr";
#endif
        Ref<ImageAsset> image = IAssetManager::singleton().findImage(path);
        if (!image) {
            return {};
        }
        m_env_texture = m_device.createTexture(image.get());
    }

    if (!m_env_cube) {
        DEV_ASSERT(!m_diffuse);
        DEV_ASSERT(!m_specular);

        {
            GpuResourceDesc desc = RenderGraph::buildDefaultTextureDesc(
                PixelFormat::R32G32B32A32_FLOAT,
                AttachmentType::COLOR_CUBE,
                kCubemapSize,
                kCubemapSize,
                6,
                RESOURCE_MISC_GENERATE_MIPS,
                kIBLMipChainMax);
            desc.bind_flags |= BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;

            m_env_cube = m_device.createTexture(desc, CubemapSampler());
        }
        {

            GpuResourceDesc desc = RenderGraph::buildDefaultTextureDesc(
                PixelFormat::R32G32B32A32_FLOAT,
                AttachmentType::COLOR_CUBE,
                kIrradianceCubemapSize,
                kIrradianceCubemapSize,
                6);
            desc.bind_flags |= BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;

            m_diffuse = m_device.createTexture(desc, CubemapNoMipSampler());
        }
        {
            GpuResourceDesc desc = RenderGraph::buildDefaultTextureDesc(
                PixelFormat::R32G32B32A32_FLOAT,
                AttachmentType::COLOR_CUBE,
                kPrefilteredCubemapSize,
                kPrefilteredCubemapSize,
                6,
                RESOURCE_MISC_GENERATE_MIPS,
                kIBLMipChainMax);
            desc.bind_flags |= BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;

            m_specular = m_device.createTexture(desc, CubemapLodSampler());
        }
    }

    RGTextureId env_hdr = graph.importResource({ m_env_texture });
    RGTextureId env_cube = graph.importResource({ m_env_cube });
    RGTextureId diffuse = graph.importResource({ m_diffuse });
    RGTextureId specular = graph.importResource({ m_specular });

    // bake environment cubemap
    for (uint16_t face = 0; face < 6; ++face) {
        std::string pass_name = std::format("{}_{}", kPassBakeSkybox, face);
        RenderPass& pass = graph.addRenderPass(pass_name);

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
        std::string pass_name = std::format("{}_{}", kPassBakeDiffuse, face);
        RenderPass& pass = graph.addRenderPass(pass_name);

        TextureViewDesc view_desc{};
        view_desc.first_array_slice = face;
        pass.read(ResourceAccess::SRV, env_cube)
            .writeColor(diffuse, view_desc, LoadOp::Load)
            .setExecuteFunc([face](RenderPassExcutionContext& ctx) {
                DiffuseIrradianceFunc(ctx, face);
            });
    }

    int w = kPrefilteredCubemapSize;
    int h = kPrefilteredCubemapSize;
    for (uint16_t mip = 0; mip < kIBLMipChainMax; ++mip, w /= 2, h /= 2) {
        for (uint16_t face = 0; face < 6; ++face) {
            TextureViewDesc view_desc{
                .mip_slice = mip,
                .first_array_slice = face,
                .array_size = 1,
            };
            std::string pass_name = std::format("{}_{}_{}", kPassBakePrefiltered, mip, face);
            RenderPass& pass = graph.addRenderPass(pass_name);
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
