#pragma once
#include "cave/render/rhi/Defines.h"
#include "cave/runtime/framework/EngineServices.h"

// @TODO: fix
#include "engine/private/render/render_graph/RenderGraphDefines.h"

// clang-format off
namespace cave { struct FrameData; }
// clang-format on

namespace cave::render {

class ICommandList;
class IRenderDevice;
struct CompiledPass;
struct GpuResource;

struct RenderPassExcutionContext {
    const FrameData& frameData;
    const CompiledPass& pass;
    ICommandList& command;
    IRenderDevice& device;
    EngineServices& services;
};

using ExecuteFunc = std::function<void(RenderPassExcutionContext& ctx)>;

struct CompiledPass {
    int pass_id;
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
