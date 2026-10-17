#pragma once
#include "cave/core/math/Vec.h"
#include "cave/render/rhi/GpuResources.h"

#include "engine/private/render/render_graph/RGTextureId.h"
#include "engine/private/runtime/framework/IRenderDevice.h"

namespace cave::render {

struct RenderOptions;
class RenderGraph;

using KernelData = std::array<math::Vec4f, 64>;

class SsaoFeature {
public:
    struct Inputs {
        RGTextureId normal{};
        RGTextureId depth{};
    };

    struct Outputs {
        RGTextureId processed{};
    };

    SsaoFeature(IRenderDevice& device)
        : m_device(device) {}

    [[nodiscard]] Outputs build(RenderGraph& graph,
                                const Inputs& in,
                                const RenderOptions& plan);

    static KernelData createKernel();

private:
    GpuTextureId m_ssao_texture{};
    IRenderDevice& m_device;
};

}  // namespace cave::render
