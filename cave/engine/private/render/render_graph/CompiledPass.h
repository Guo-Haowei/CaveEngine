#pragma once
#include "cave/runtime/framework/EngineServices.h"

// @TODO: fix
#include "engine/private/render/render_graph/RenderGraphDefines.h"
#include "engine/private/render/rhi/RenderTarget.h"
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
    EngineServices& services;
};

using ExecuteFunc = std::function<void(RenderPassExcutionContext& ctx)>;

struct CompiledPass {
    String name;

    Vector<ColorAttachmentDesc> colors;
    std::optional<DepthAttachmentDesc> depth;

    std::optional<Viewport> viewport;
    // @TODO: sissor

    Vector<GpuTextureId> uavs;
    Vector<GpuTextureId> srvs;

    ExecuteFunc execute_func;
};

}  // namespace cave::render
