#include "SsaoFeature.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/core/math/Vec.h"
#include "cave/render/rhi/Sampler.h"

// @TODO: remove this
#include "engine/private/render/render_graph/RenderGraph.h"
#include "engine/private/render/renderer/FrameData.h"
#include "engine/private/core/base/random.h"
#include "engine/private/render/rhi/PipelineState.h"

namespace cave {
#include "shader_resource_defines.slang.h"
}  // namespace cave

namespace cave::render {

using math::Vec2f;
using math::Vec3f;
using math::Vec4f;

static_assert(sizeof(KernelData) == sizeof(Vec4f) * SSAO_KERNEL_SIZE);

static GpuTextureId GenerateSsaoNoise(IRenderDevice& p_device) {
    // generate noise texture
    std::vector<Vec2f> ssao_noise;
    for (int i = 0; i < (SSAO_NOISE_SIZE * SSAO_NOISE_SIZE); ++i) {
        Vec2f noise(Random::Float(-1.0f, 1.0f),
                    Random::Float(-1.0f, 1.0f));
        ssao_noise.emplace_back(noise);
    }

    GpuResourceDesc desc{
        .dimension = Dimension::Texture2D,
        .width = SSAO_NOISE_SIZE,
        .height = SSAO_NOISE_SIZE,
        .depth = 1,
        .mip_levels = 1,
        .array_size = 1,
        .format = PixelFormat::R32G32_FLOAT,
        .bind_flags = BIND_SHADER_RESOURCE,
        .misc_flags = RESOURCE_MISC_NONE,
        .initial_data = ssao_noise.data(),

        .type = AttachmentType::NONE,
    };

    return p_device.createTexture(desc, PointWrapSampler());
}

static void SsaoPassFunc(RenderPassExcutionContext& ctx) {
    CAVE_PROFILE_EVENT();
    if (!ctx.frameData.options.enable_ssao) {
        return;
    }

    auto& cmd = ctx.command;

    cmd.setPipelineState(PSO_SSAO);
    cmd.setMesh(nullptr);
    cmd.draw(6);
}

SsaoFeature::Outputs SsaoFeature::build(RenderGraph& graph,
                                        const Inputs& inputs,
                                        const RenderOptions&) {
    constexpr const char kPassSsao[] = "p:ssao";
    constexpr const char kResSsao[] = "r:ssao";

    if (!m_ssao_texture) {
        m_ssao_texture = GenerateSsaoNoise(m_device);
    }

    RGTextureId noise = graph.importTexture({ m_ssao_texture });

    RenderPass& pass = graph.addRenderPass(kPassSsao);
    Outputs out{
        .processed = graph.createTexture({
            kResSsao,
            graph.buildDefaultTextureDesc(kFormatSsao, AttachmentType::COLOR_2D),
        })
    };

    pass.read(ResourceAccess::SRV, inputs.normal)
        .read(ResourceAccess::SRV, inputs.depth)
        .read(ResourceAccess::SRV, noise)
        .writeColor(out.processed, {}, LoadOp::Clear)
        .setExecuteFunc(SsaoPassFunc);

    return out;
}

KernelData SsaoFeature::createKernel() {
    auto lerp = [](float a, float b, float f) {
        return a + f * (b - a);
    };

    KernelData kernel;

    const int kernel_size = 32;
    const float inv_kernel_size = 1.0f / kernel_size;
    for (int i = 0; i < static_cast<int>(kernel.size()); ++i) {
        // [-1, 1], [-1, 1], [0, 1]
        Vec3f sample(Random::Float(-1.0f, 1.0f),
                     Random::Float(-1.0f, 1.0f),
                     Random::Float());

        sample = normalize(sample);
        sample *= Random::Float();
        float scale = i * inv_kernel_size;

        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        kernel[i].xyz = sample;
    }

    return kernel;
}
}  // namespace cave::render
