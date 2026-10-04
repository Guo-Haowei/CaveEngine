#pragma once
#include "cave/core/diagnostics/Command.h"
#include "cave/core/time/FrameTime.h"
#include "cave/runtime/framework/IService.h"

#include "engine/private/render/renderer/CanvasRenderer.h"
#include "engine/private/runtime/view/ResolvedView.h"

// clang-format off
namespace cave { class ICanvas; }
namespace cave { struct EngineServices; }
namespace cave { struct FrameData; }
// clang-format on

namespace cave::render {

struct RendererCapabilities {
    bool canRunBloom = false;
    bool canRunIBL = false;
};

class UIRenderer : public CanvasRenderer {
public:
    using CanvasRenderer::CanvasRenderer;
};

class OverlayRenderer : public CanvasRenderer {
public:
    using CanvasRenderer::CanvasRenderer;
};

class Renderer : public IService {
public:
    Renderer(EngineServices& services);
    ~Renderer();

    void tick(const FrameTime& frame, std::span<const ResolvedView> views);

    const RendererCapabilities& getCapabilities() const;
    // @TODO: instead, create renderer after project selected
    void setMode(bool is_2d);

    template<typename T>
    T* tryGet() { return nullptr; }
    template<>
    OverlayRenderer* tryGet() { return m_overlay_renderer.get(); }
    template<>
    UIRenderer* tryGet() { return m_ui_renderer.get(); }

#if USING(USE_COMMAND)
    bool cmdDump(CommandContext& ctx, const CommandArgs& args);
#endif

protected:
    auto InitializeImpl() -> Result<void> final;
    void FinalizeImpl() final;

private:
    class Impl;

    Owner<Impl> m_impl;

    // renderers
    Owner<OverlayRenderer> m_overlay_renderer;
    Owner<UIRenderer> m_ui_renderer;
};

}  // namespace cave::render
