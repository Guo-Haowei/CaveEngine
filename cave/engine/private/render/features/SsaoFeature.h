#pragma once
#include "cave/core/math/Vector.h"

#include "engine/private/render/render_graph/RGTextureId.h"
#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/runtime/framework/IRenderDevice.h"

namespace cave::render {

struct FramePlan;
class RenderGraphBuilder;

using KernelData = std::array<math::Vector4f, 64>;

class SsaoFeature {
public:
    struct Inputs {
        RGTextureId normal{};
        RGTextureId depth{};
    };

    struct Outputs {
        RGTextureId processed{};
    };

    SsaoFeature(IRenderDevice& p_device)
        : m_device(p_device) {}

    [[nodiscard]] Outputs Build(RenderGraphBuilder& p_builder,
                                const FramePlan& p_plan,
                                const Inputs& p_in);

    static KernelData CreateKernel();

private:
    GpuTextureId m_ssao_texture{};
    IRenderDevice& m_device;
};

}  // namespace cave::render
