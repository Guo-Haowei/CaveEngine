#pragma once
#include "cave/core/diagnostics/Command.h"
#include "cave/core/time/FrameTime.h"

#include "engine/private/runtime/view/ResolvedView.h"

// clang-format off
namespace cave { class ICanvas; }
namespace cave { struct FrameData; }
namespace cave { struct UIFrameDrawData; }
// clang-format on

namespace cave::render {

class IRenderDevice;

class Renderer {
public:
    Renderer(IRenderDevice& device, ICanvas& debug_draw);
    ~Renderer();

    void tick(const FrameTime& frame,
              std::span<const ResolvedView> views,
              const UIFrameDrawData& ui_data);

    // @TODO: instead, create renderer after project selected
    void setMode(bool is_2d);

#if USING(USE_COMMAND)
    bool Cmd_dump(CommandContext& ctx, const CommandArgs& args);
#endif

private:
    class Impl;

    std::unique_ptr<Impl> impl_;
};

}  // namespace cave::render
