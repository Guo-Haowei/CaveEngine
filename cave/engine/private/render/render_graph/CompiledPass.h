#pragma once
#include "engine/private/render/render_graph/RenderGraphDefines.h"
#include "engine/private/render/rhi/RenderTarget.h"

// @TODO: fix
#include "engine/private/renderer/graphics_defines.h"

// clang-format off
namespace cave { struct FrameData; }
namespace cave { struct GpuTexture; }
// clang-format on

namespace cave::render {

class IRenderDevice;
struct CompiledPass;

struct RenderPassExcutionContext {
    const FrameData& frameData;
    const CompiledPass& pass;
    IRenderDevice& cmd;
};

using ExecuteFunc = std::function<void(RenderPassExcutionContext& ctx)>;

struct CompiledPass {
    std::string name;

    std::vector<ColorAttachmentDesc> colors;
    std::optional<DepthAttachmentDesc> depth;

    std::optional<Viewport> viewport;
    // @TODO: sissor

    std::vector<GpuTextureId> uavs;
    std::vector<GpuTextureId> srvs;

    ExecuteFunc func;
};

}  // namespace cave::render
