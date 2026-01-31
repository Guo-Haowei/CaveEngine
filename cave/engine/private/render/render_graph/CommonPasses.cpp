#include "CommonPasses.h"

#include "engine/private/algorithm/algorithm.h"
#include "engine/private/assets/image_asset.h"
#include "engine/private/core/base/random.h"
#include "engine/private/core/debugger/Profiler.h"
#include "engine/private/renderer/frame_data.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/renderer/graphics_manager.h"
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

using math::Vector2f;

constexpr const char RG_PASS_2D[] = "p:2d";
constexpr const char RG_PASS_DEPTH_PREPASS[] = "p:early_z";
constexpr const char RG_PASS_GBUFFER[] = "p:gbuffer";
constexpr const char RG_PASS_VOXELIZATION[] = "p:voxelization";
constexpr const char RG_PASS_LIGHTING[] = "p:lighting";
constexpr const char RG_PASS_FORWARD[] = "p:forward";
constexpr const char RG_PASS_BLOOM_SETUP[] = "p:bloom_setup";
constexpr const char RG_PASS_POST_PROCESS[] = "p:post_process";
constexpr const char RG_PASS_SSAO[] = "p:ssao";
constexpr const char RG_PASS_OUTLINE[] = "p:outline";
constexpr const char RG_PASS_PATHTRACER[] = "p:pathtracer";
constexpr const char RG_PASS_PATHTRACER_PRESENT[] = "p:pathtracer_present";
constexpr const char RG_PASS_BAKE_SKYBOX[] = "p:env_skybox";
constexpr const char RG_PASS_BAKE_DIFFUSE[] = "p:diffuse";
constexpr const char RG_PASS_BAKE_PREFILTERED[] = "p:prefiltered";

constexpr const char RG_RES_DEPTH_STENCIL[] = "r:depth";
constexpr const char RG_RES_GBUFFER_COLOR0[] = "r:gbuffer0";
constexpr const char RG_RES_GBUFFER_COLOR1[] = "r:gbuffer1";
constexpr const char RG_RES_GBUFFER_COLOR2[] = "r:gbuffer2";
constexpr const char RG_RES_LIGHTING[] = "r:lighting";
constexpr const char RG_RES_SSAO[] = "r:ssao";
constexpr const char RG_RES_POST_PROCESS[] = "r:post_process";
constexpr const char RG_RES_VOXEL_LIGHTING[] = "r:voxel_lighting";
constexpr const char RG_RES_VOXEL_NORMAL[] = "r:voxel_normal";
constexpr const char RG_RES_OUTLINE[] = "r:outline";
constexpr const char RG_RES_PATHTRACER[] = "r:pathtracer";
constexpr const char RG_RES_ENV_SKYBOX_CUBE[] = "r:env_cube";
constexpr const char RG_RES_ENV_DIFFUSE_CUBE[] = "r:diffuse_cube";
constexpr const char RG_RES_ENV_PREFILTERED_CUBE[] = "r:prefiltered_cube";

// external resources
constexpr const char RG_RES_SSAO_NOISE[] = "r:ssao_noise";
constexpr const char RG_RES_BRDF[] = "r:brdf";
constexpr const char RG_RES_IBL[] = "r:ibl";
constexpr const char RG_RES_LTC1[] = "r:ltc1";
constexpr const char RG_RES_LTC2[] = "r:ltc2";

static std::shared_ptr<GpuTexture> GenerateSsaoNoise() {
    // generate noise texture
    std::vector<Vector2f> ssao_noise;
    for (int i = 0; i < (SSAO_NOISE_SIZE * SSAO_NOISE_SIZE); ++i) {
        Vector2f noise(Random::Float(-1.0f, 1.0f),
                       Random::Float(-1.0f, 1.0f));
        ssao_noise.emplace_back(noise);
    }

    GpuTextureDesc desc{
        .type = AttachmentType::NONE,
        .dimension = Dimension::TEXTURE_2D,
        .width = SSAO_NOISE_SIZE,
        .height = SSAO_NOISE_SIZE,
        .depth = 1,
        .mipLevels = 1,
        .arraySize = 1,
        .format = PixelFormat::R32G32_FLOAT,
        .bindFlags = BIND_SHADER_RESOURCE,
        .miscFlags = RESOURCE_MISC_NONE,
        .initialData = ssao_noise.data(),
        .name = RG_RES_SSAO,
    };

    return GraphicsManager::GetSingleton().CreateTexture(desc, PointWrapSampler());
}

static std::shared_ptr<GpuTexture> GenerateLTC(std::string_view p_name, const float* p_matrix_table) {
    constexpr int LTC_SIZE = 64;
    GpuTextureDesc desc{
        .type = AttachmentType::NONE,
        .dimension = Dimension::TEXTURE_2D,
        .width = LTC_SIZE,
        .height = LTC_SIZE,
        .depth = 1,
        .mipLevels = 1,
        .arraySize = 1,
        .format = PixelFormat::R32G32B32A32_FLOAT,
        .bindFlags = BIND_SHADER_RESOURCE,
        .miscFlags = RESOURCE_MISC_NONE,
        .initialData = p_matrix_table,
        .name = std::string(p_name),
    };

    return GraphicsManager::GetSingleton().CreateTexture(desc, PointClampSampler());
}

extern void DepthPrepassFunc(RenderPassExcutionContext& p_ctx);
extern void GbufferPassFunc(RenderPassExcutionContext& p_ctx);
extern void SsaoPassFunc(RenderPassExcutionContext& p_ctx);
extern void HighlightPassFunc(RenderPassExcutionContext& p_ctx);
extern void VoxelizationPassFunc(RenderPassExcutionContext& p_ctx);
extern void LightingPassFunc(RenderPassExcutionContext& p_ctx);
extern void ForwardPassFunc(RenderPassExcutionContext& p_ctx);
extern void BloomSetupFunc(RenderPassExcutionContext& p_ctx);
extern void BloomDownSampleFunc(RenderPassExcutionContext& p_ctx);
extern void BloomUpSampleFunc(RenderPassExcutionContext& p_ctx);
extern void TonePassFunc(RenderPassExcutionContext& p_ctx);
extern void ConvertToCubemapFunc(RenderPassExcutionContext& p_ctx);
extern void DiffuseIrradianceFunc(RenderPassExcutionContext& p_ctx);
extern void PrefilteredFunc(RenderPassExcutionContext& p_ctx);
extern void PathTracerPassFunc(RenderPassExcutionContext& p_ctx);
extern void PathTracerTonePassFunc(RenderPassExcutionContext& p_ctx);

DepthPrepassOutput RenderGraphBuilderExt::AddDepthPrepass() {
    RenderPassBuilder& pass = AddPass(RG_PASS_DEPTH_PREPASS);

    DepthPrepassOutput out{
        .depth = CreateTexture({
            RG_RES_DEPTH_STENCIL,
            BuildDefaultTextureDesc(RT_FMT_GBUFFER_DEPTH, AttachmentType::DEPTH_STENCIL_2D),
        }),
    };

    pass.Write(ResourceAccess::DSV, out.depth)
        .SetExecuteFunc(DepthPrepassFunc);

    return out;
}

GbufferOutput RenderGraphBuilderExt::AddGbufferPass(const DepthPrepassOutput& p_in) {
    RenderPassBuilder& pass = AddPass(RG_PASS_GBUFFER);

    GbufferOutput out{
        .color0 = CreateTexture(RGResourceCreateDesc{
            RG_RES_GBUFFER_COLOR0,
            BuildDefaultTextureDesc(RT_FMT_GBUFFER_BASE_COLOR, AttachmentType::COLOR_2D),
        }),
        .color1 = CreateTexture(RGResourceCreateDesc{
            RG_RES_GBUFFER_COLOR1,
            BuildDefaultTextureDesc(RT_FMT_GBUFFER_NORMAL, AttachmentType::COLOR_2D),
        }),
        .color2 = CreateTexture(RGResourceCreateDesc{
            RG_RES_GBUFFER_COLOR2,
            BuildDefaultTextureDesc(RT_FMT_GBUFFER_MATERIAL, AttachmentType::COLOR_2D),
        }),
    };

    // @TODO: introduce versioning
    pass.Read(ResourceAccess::DSV, p_in.depth)
        .Write(ResourceAccess::DSV, p_in.depth)
        .Write(ResourceAccess::RTV, out.color0)
        .Write(ResourceAccess::RTV, out.color1)
        .Write(ResourceAccess::RTV, out.color2)
        .SetExecuteFunc(GbufferPassFunc);
    return out;
}

SsaoOutput RenderGraphBuilderExt::AddSsaoPass(const SsaoInput& p_in) {
    RGTextureId noise = ImportTexture({
        .debug_name = RG_RES_SSAO_NOISE,
        .func = []() { return GenerateSsaoNoise(); },
    });

    RenderPassBuilder& pass = AddPass(RG_PASS_SSAO);
    SsaoOutput out{
        .processed = CreateTexture({
            RG_RES_SSAO,
            BuildDefaultTextureDesc(RT_FMT_SSAO, AttachmentType::COLOR_2D),
        })
    };

    pass.Read(ResourceAccess::SRV, p_in.normal)
        .Read(ResourceAccess::SRV, p_in.depth)
        .Read(ResourceAccess::SRV, noise)
        .Write(ResourceAccess::RTV, out.processed)
        .SetExecuteFunc(SsaoPassFunc);
    return out;
}

LightingOutput RenderGraphBuilderExt::AddLightingPass(const LightingInput& p_in) {
    RGTextureId brdf = ImportTexture(RGResourceImportDesc{
        .debug_name = RG_RES_BRDF,
        .func = []() {
            std::shared_ptr<ImageAsset> image = IAssetManager::GetSingleton().FindImage("brdf.hdr");
            return GraphicsManager::GetSingleton().CreateTexture(image.get());
        },
    });

    RGTextureId ltc1 = ImportTexture(RGResourceImportDesc{
        .debug_name = RG_RES_LTC1,
        .func = []() { return GenerateLTC(RG_RES_LTC1, LTC1); },
    });

    RGTextureId ltc2 = ImportTexture(RGResourceImportDesc{
        .debug_name = RG_RES_LTC2,
        .func = []() { return GenerateLTC(RG_RES_LTC2, LTC2); },
    });

    RGTextureId out = CreateTexture(RGResourceCreateDesc{
        RG_RES_LIGHTING,
        BuildDefaultTextureDesc(RT_FMT_LIGHTING, AttachmentType::COLOR_2D),
    });

    RenderPassBuilder& pass = AddPass(RG_PASS_LIGHTING);

    pass.Read(ResourceAccess::SRV, p_in.color0)
        .Read(ResourceAccess::SRV, p_in.color1)
        .Read(ResourceAccess::SRV, p_in.color2)
        .Read(ResourceAccess::SRV, p_in.depth)
        .Read(ResourceAccess::SRV, p_in.ssao)
        .Read(ResourceAccess::SRV, p_in.shadow)
        .Read(ResourceAccess::SRV, p_in.ibl_diffuse)
        .Read(ResourceAccess::SRV, p_in.ibl_prefiltered)
        .Read(ResourceAccess::SRV, brdf)
        .Read(ResourceAccess::SRV, ltc1)
        .Read(ResourceAccess::SRV, ltc2)
        .Write(ResourceAccess::RTV, out)
        .SetExecuteFunc(LightingPassFunc);

    return { out };

#if 0
    if (m_config.enableVxgi) {
        pass.Read(ResourceAccess::SRV, RG_RES_VOXEL_LIGHTING)
            .Read(ResourceAccess::SRV, RG_RES_VOXEL_NORMAL);
    }
#endif
}

PostProcessOutput RenderGraphBuilderExt::AddPostProcessPass(const PostProcessInput& p_in) {
    RenderPassBuilder& pass = AddPass(RG_PASS_POST_PROCESS);
    auto desc = BuildDefaultTextureDesc(RT_FMT_TONE, AttachmentType::COLOR_2D);
    desc.bindFlags |= BIND_SHADER_RESOURCE;

    PostProcessOutput out{
        .processed = CreateTexture(RGResourceCreateDesc{
            RG_RES_POST_PROCESS,
            desc,
        }),
    };

    pass.Read(ResourceAccess::SRV, p_in.lighting)
        .Read(ResourceAccess::SRV, p_in.outline)
        .Read(ResourceAccess::SRV, p_in.bloom);

    pass.Write(ResourceAccess::RTV, out.processed)
        .SetExecuteFunc(TonePassFunc);

    return out;
}
#if 0
void RenderGraphBuilderExt::AddHighlightPass() {
    RenderPassBuilder& pass = AddPass(RG_PASS_OUTLINE);



    auto color0_desc = BuildDefaultTextureDesc(RT_FMT_OUTLINE_SELECT,
                                               AttachmentType::COLOR_2D);

    pass.Create(RG_RES_OUTLINE, { color0_desc })
        .Write(ResourceAccess::RTV, RG_RES_OUTLINE)
        .Write(ResourceAccess::DSV, RG_RES_DEPTH_STENCIL)
        .SetExecuteFunc(HighlightPassFunc);
}

void RenderGraphBuilderExt::AddVoxelizationPass() {
    auto& manager = m_graphicsManager;
    if (manager.GetBackend() != Backend::OPENGL) {
        return;
    }

    const int voxel_size = DVAR_GET_INT(gfx_voxel_size);
    GpuTextureDesc desc = BuildDefaultTextureDesc(PixelFormat::R16G16B16A16_FLOAT,
                                                  AttachmentType::RW_TEXTURE,
                                                  voxel_size, voxel_size);
    desc.dimension = Dimension::TEXTURE_3D;
    desc.mipLevels = LogTwo(voxel_size);
    desc.depth = voxel_size;
    desc.miscFlags |= RESOURCE_MISC_GENERATE_MIPS;

    SamplerDesc sampler(MinFilter::LINEAR_MIPMAP_LINEAR, MagFilter::POINT, AddressMode::BORDER);

    auto& pass = AddPass(RG_PASS_VOXELIZATION);
    pass.Create(RG_RES_VOXEL_LIGHTING, { desc, sampler })
        .Create(RG_RES_VOXEL_NORMAL, { desc, sampler })
        .Read(ResourceAccess::SRV, RG_RES_SHADOW_MAP)
        //.Read(ResourceAccess::SRV, RG_RES_LTC1)
        //.Read(ResourceAccess::SRV, RG_RES_LTC2)
        .Read(ResourceAccess::UAV, RG_RES_VOXEL_LIGHTING)
        .Read(ResourceAccess::UAV, RG_RES_VOXEL_NORMAL)
        .SetExecuteFunc(VoxelizationPassFunc);
}

void RenderGraphBuilderExt::AddForwardPass() {
    auto& pass = AddPass(RG_PASS_FORWARD);
    AddDependency(RG_PASS_LIGHTING, RG_PASS_FORWARD);
    pass.Read(ResourceAccess::SRV, RG_RES_ENV_SKYBOX_CUBE)
        .Read(ResourceAccess::SRV, RG_RES_SHADOW_MAP)
        .Read(ResourceAccess::SRV, RG_RES_ENV_DIFFUSE_CUBE)
        .Read(ResourceAccess::SRV, RG_RES_ENV_PREFILTERED_CUBE)
        .Read(ResourceAccess::SRV, RG_RES_BRDF)
        .Read(ResourceAccess::SRV, RG_RES_LTC1)
        .Read(ResourceAccess::SRV, RG_RES_LTC2)
        .Write(ResourceAccess::DSV, RG_RES_DEPTH_STENCIL)
        .Write(ResourceAccess::RTV, RG_RES_LIGHTING)
        .SetExecuteFunc(ForwardPassFunc);

    if (m_config.enableVxgi) {
        pass.Read(ResourceAccess::SRV, RG_RES_VOXEL_LIGHTING)
            .Read(ResourceAccess::SRV, RG_RES_VOXEL_NORMAL);
    }
}
#endif

/// Bloom

#if 0
void RenderGraphBuilderExt::AddBloomPass() {
    // Setup pass
    const int width = m_config.frameWidth;
    const int height = m_config.frameHeight;

    auto& setup_pass = AddPass(RG_PASS_BLOOM_SETUP);
    SamplerDesc sampler = LinearClampSampler();

    // @TODO: use mips instead of generate this many resources
    for (int i = 0, w = width, h = height; i < BLOOM_MIP_CHAIN_MAX; ++i, w /= 2, h /= 2) {
        DEV_ASSERT(width > 1);
        DEV_ASSERT(height > 1);

        auto texture_desc = BuildDefaultTextureDesc(PixelFormat::R16G16B16A16_FLOAT,
                                                    AttachmentType::COLOR_2D,
                                                    w, h);

        auto res_name = std::format(RG_RES_BLOOM_PREFIX "{}x{}", w, h);
        setup_pass.Create(res_name, { texture_desc, sampler });
    }

    auto bloom_res = std::format(RG_RES_BLOOM_PREFIX "{}x{}", width, height);

    AddDependency(RG_PASS_FORWARD, RG_PASS_BLOOM_SETUP);
    setup_pass
        .Read(ResourceAccess::SRV, RG_RES_LIGHTING)
        .Read(ResourceAccess::UAV, bloom_res)
        .SetExecuteFunc(BloomSetupFunc);

    // Down Sample
    for (int i = 0, w = width, h = height; i < BLOOM_MIP_CHAIN_MAX - 1; ++i, w /= 2, h /= 2) {
        auto pass_name = std::format(RG_PASS_BLOOM_DOWN_PREFIX "{}", i);
        auto input = std::format(RG_RES_BLOOM_PREFIX "{}x{}", w, h);
        auto inout = std::format(RG_RES_BLOOM_PREFIX "{}x{}", w / 2, h / 2);
        auto& pass = AddPass(pass_name);
        pass.Read(ResourceAccess::SRV, input)
            .Read(ResourceAccess::UAV, inout)
            .SetExecuteFunc(BloomDownSampleFunc);
        if (i == 0) {
            AddDependency(RG_PASS_BLOOM_SETUP, pass_name);
        } else {
            std::string prev_pass = std::format(RG_PASS_BLOOM_DOWN_PREFIX "{}", i - 1);
            AddDependency(prev_pass, pass_name);
        }
    }

    // Up Sample
    for (int i = 0, w = width, h = height; i < BLOOM_MIP_CHAIN_MAX - 1; ++i, w /= 2, h /= 2) {
        auto pass_name = std::format(RG_PASS_BLOOM_UP_PREFIX "{}", i);
        auto mip_low = std::format(RG_RES_BLOOM_PREFIX "{}x{}", w / 2, h / 2);
        auto mip = std::format(RG_RES_BLOOM_PREFIX "{}x{}", w, h);

        auto& pass = AddPass(pass_name);
        pass.Read(ResourceAccess::UAV, mip)
            .Read(ResourceAccess::SRV, mip_low)
            .SetExecuteFunc(BloomUpSampleFunc);

        if (i == BLOOM_MIP_CHAIN_MAX - 2) {
            auto down_sample_pass = std::format(RG_PASS_BLOOM_DOWN_PREFIX "{}", BLOOM_MIP_CHAIN_MAX - 2);
            AddDependency(down_sample_pass, pass_name);
        } else {
            auto prev_pass = std::format(RG_PASS_BLOOM_UP_PREFIX "{}", i + 1);
            AddDependency(prev_pass, pass_name);
        }
    }
}


void RenderGraphBuilderExt::AddGenerateSkylightPass() {
    {
        GpuTextureDesc desc = BuildDefaultTextureDesc(PixelFormat::R32G32B32A32_FLOAT,
                                                      AttachmentType::COLOR_CUBE,
                                                      RT_SIZE_IBL_CUBEMAP,
                                                      RT_SIZE_IBL_CUBEMAP,
                                                      6,
                                                      RESOURCE_MISC_GENERATE_MIPS,
                                                      IBL_MIP_CHAIN_MAX);

        auto& pass = AddPass(RG_PASS_BAKE_SKYBOX);
        pass.Import(RG_RES_IBL, []() {
                std::shared_ptr<ImageAsset> image = IAssetManager::GetSingleton().FindImage("sky.hdr");
                return GraphicsManager::GetSingleton().CreateTexture(image.get());
            })
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

void RenderGraphBuilderExt::AddPathTracerPass() {
#if 0
    GpuTextureDesc texture_desc = BuildDefaultTextureDesc(PixelFormat::R32G32B32A32_FLOAT,
                                                          AttachmentType::COLOR_2D);

    auto& pass = AddPass(RG_PASS_PATHTRACER);
    pass.Create(RG_RES_PATHTRACER, { texture_desc, LinearClampSampler() })
        .Read(ResourceAccess::UAV, RG_RES_PATHTRACER)
        .SetExecuteFunc(PathTracerPassFunc);
#endif
}

void RenderGraphBuilderExt::AddPathTracerTonePass() {
#if 0
    auto& pass = AddPass(RG_PASS_PATHTRACER_PRESENT);

    pass.Import(RG_RES_POST_PROCESS, []() {
            return GraphicsManager::GetSingleton().FindTexture(RG_RES_POST_PROCESS);
        })
        .Read(ResourceAccess::SRV, RG_RES_PATHTRACER)
        .Write(ResourceAccess::RTV, RG_RES_POST_PROCESS)
        .Write(ResourceAccess::DSV, RG_RES_DEPTH_STENCIL)
        .SetExecuteFunc(PathTracerTonePassFunc);
#endif
}

#if 0
auto RenderGraphBuilderExt::CreatePathTracer(RenderGraphBuilderConfig& p_config) -> Result<std::shared_ptr<RenderGraph>> {
    p_config.enableBloom = false;
    p_config.enableIbl = false;
    p_config.enableVxgi = false;
    p_config.enableHighlight = false;

    RenderGraphBuilderExt creator(p_config);

    creator.AddPathTracerPass();
    creator.AddPathTracerTonePass();

    return creator.Compile();
}
#endif

}  // namespace cave::render
