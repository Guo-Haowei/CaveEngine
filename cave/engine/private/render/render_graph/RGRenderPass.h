#pragma once
#include "engine/private/render/render_graph/RenderGraphDefines.h"

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

    std::vector<std::shared_ptr<GpuTexture>> rtvs;
    std::shared_ptr<GpuTexture> dsv;

    // @TODO: remove these
    std::vector<std::shared_ptr<GpuTexture>> uavs;
    std::vector<std::shared_ptr<GpuTexture>> srvs;

    std::shared_ptr<RenderTarget> framebuffer;

    ExecuteFunc func;
};

}  // namespace cave::render
