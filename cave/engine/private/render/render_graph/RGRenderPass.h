#pragma once
#include "engine/private/render/render_graph/RenderGraphDefines.h"
#include "engine/private/render/rhi/RenderTarget.h"

// clang-format off
namespace cave { struct FrameData; }
namespace cave { struct GpuTexture; }
// clang-format on

namespace cave::render {

class IRenderDevice;
struct RenderTarget;
struct RGRenderPass;

struct RenderPassExcutionContext {
    const FrameData& frameData;
    RenderTarget* framebuffer;
    RGRenderPass& pass;
    IRenderDevice& cmd;
};

using ExecuteFunc = void (*)(RenderPassExcutionContext& ctx);

struct RGRenderPass {
    std::string name;

    std::vector<ColorAttachmentDesc> colors;
    DepthAttachmentDesc depth;

    // @TODO: replace framebuffer with RenderTargetDesc
    std::shared_ptr<RenderTarget> framebuffer;

    std::optional<Viewport> viewport;
    // @TODO: sissor

    std::vector<std::shared_ptr<GpuTexture>> uavs;
    std::vector<std::shared_ptr<GpuTexture>> srvs;

    ExecuteFunc func;
};

}  // namespace cave::render
