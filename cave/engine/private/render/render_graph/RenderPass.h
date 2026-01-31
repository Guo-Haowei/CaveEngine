#pragma once
#include "engine/private/render/render_graph/RenderGraphDefines.h"

// clang-format off
namespace cave { class IRenderDevice; }
namespace cave { struct GpuTexture; }
// clang-format on

namespace cave {
// @TODO: split RHI and RenderCommandContext
using IRenderCmdContext = IRenderDevice;
struct FrameData;
class RenderDevice;
}  // namespace cave

namespace cave::render {

class RenderPass;
struct Framebuffer;

struct RenderPassExcutionContext {
    const FrameData& frameData;
    Framebuffer* framebuffer;
    RenderPass& pass;
    IRenderCmdContext& cmd;
};

using ExecuteFunc = void (*)(RenderPassExcutionContext& ctx);

class RenderPass {
public:
    std::string_view GetName() const { return m_name; }

    const auto& GetUavs() const { return m_uavs; }
    const auto& GetRtvs() const { return m_rtvs; }
    const auto& GetSrvs() const { return m_srvs; }
    const auto& GetDsv() const { return m_dsv; }

protected:
    std::string m_name;

    std::vector<std::shared_ptr<GpuTexture>> m_rtvs;
    std::shared_ptr<GpuTexture> m_dsv;

    std::vector<std::shared_ptr<GpuTexture>> m_uavs;
    std::vector<std::shared_ptr<GpuTexture>> m_srvs;

    std::shared_ptr<Framebuffer> m_framebuffer;

    ExecuteFunc m_executor;

    friend class RenderPassBuilder;
    friend class RenderGraphBuilder;
    friend class RenderGraph;
    friend class RenderDevice;
};

}  // namespace cave::render
