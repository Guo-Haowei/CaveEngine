#pragma once
#include "cave/core/diagnostics/Command.h"
#include "cave/core/time/FrameTime.h"

#include "engine/private/render/renderer/CanvasRenderer.h"
#include "engine/private/runtime/view/ResolvedView.h"

// clang-format off
namespace cave { class ICanvas; }
namespace cave { struct EngineServices; }
namespace cave { struct FrameData; }
namespace cave { struct UIFrameDrawData; }
// clang-format on

namespace cave::render {

class Renderer {
public:
    Renderer(EngineServices& services);
    ~Renderer();

    void tick(const FrameTime& frame,
              std::span<const ResolvedView> views,
              const UIFrameDrawData& ui_data);

    // @TODO: instead, create renderer after project selected
    void setMode(bool is_2d);

    template<typename T>
    T* tryGet() { return nullptr; }
    template<>
    CanvasRenderer* tryGet() { return m_canvas_render.get(); }

#if USING(USE_COMMAND)
    bool Cmd_dump(CommandContext& ctx, const CommandArgs& args);
#endif

private:
    class Impl;

    Owner<Impl> m_impl;

    // renderers
    Owner<CanvasRenderer> m_canvas_render;
    // @TODO: UI renderer
};

}  // namespace cave::render
