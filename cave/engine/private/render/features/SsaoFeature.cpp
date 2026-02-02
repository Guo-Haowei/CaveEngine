#include "SsaoFeature.h"

#include "cave/core/math/Vector.h"

#include "engine/private/core/debugger/Profiler.h"
#include "engine/private/render/render_graph/RenderGraphBuilder.h"

// @TODO: remove this
#include "engine/private/renderer/frame_data.h"
#include "engine/private/core/base/random.h"
#include "engine/private/renderer/sampler.h"
#include "engine/private/renderer/pipeline_state.h"

namespace cave {
#include "shader_resource_defines.hlsl.h"
}  // namespace cave

namespace cave::render {

using math::Vector2f;
using math::Vector3f;
using math::Vector4f;

static_assert(sizeof(KernelData) == sizeof(Vector4f) * SSAO_KERNEL_SIZE);

static GpuTextureId GenerateSsaoNoise(IRenderDevice& p_device) {
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
    };

    return p_device.CreateTexture(desc, PointWrapSampler());
}

static void SsaoPassFunc(RenderPassExcutionContext& p_ctx) {
    CAVE_PROFILE_EVENT();
    if (!p_ctx.frameData.options.ssaoEnabled) {
        return;
    }

    auto& cmd = p_ctx.cmd;

    cmd.SetPipelineState(PSO_SSAO);
    cmd.SetMesh(nullptr);
    cmd.DrawArrays(6);
}

SsaoFeature::Outputs SsaoFeature::Build(RenderGraphBuilder& p_builder,
                                        const FramePlan& p_plan,
                                        const Inputs& p_in) {
    unused(p_plan);

    constexpr const char RG_PASS_SSAO[] = "p:ssao";
    constexpr const char RG_RES_SSAO[] = "r:ssao";
    constexpr const char RG_RES_SSAO_NOISE[] = "r:ssao_noise";

    if (!m_ssao_texture) {
        m_ssao_texture = GenerateSsaoNoise(m_device);
    }

    RGTextureId noise = p_builder.ImportTexture({ m_ssao_texture });

    RenderPassBuilder& pass = p_builder.AddPass(RG_PASS_SSAO);
    Outputs out{
        .processed = p_builder.CreateTexture({
            RG_RES_SSAO,
            p_builder.BuildDefaultTextureDesc(RT_FMT_SSAO, AttachmentType::COLOR_2D),
        })
    };

    pass.Read(ResourceAccess::SRV, p_in.normal)
        .Read(ResourceAccess::SRV, p_in.depth)
        .Read(ResourceAccess::SRV, noise)
        .WriteColor(out.processed, {}, LoadOp::Clear)
        .SetExecuteFunc(SsaoPassFunc);

    return out;
}

KernelData SsaoFeature::CreateKernel() {
    auto lerp = [](float a, float b, float f) {
        return a + f * (b - a);
    };

    KernelData kernel;

    const int kernel_size = 32;
    const float inv_kernel_size = 1.0f / kernel_size;
    for (int i = 0; i < static_cast<int>(kernel.size()); ++i) {
        // [-1, 1], [-1, 1], [0, 1]
        Vector3f sample(Random::Float(-1.0f, 1.0f),
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
