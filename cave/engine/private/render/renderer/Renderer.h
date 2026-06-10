#pragma once
#include "cave/core/diagnostics/Command.h"
#include "cave/core/time/FrameTime.h"
#include "cave/runtime/framework/IService.h"

#include "engine/private/runtime/view/ResolvedView.h"

// clang-format off
namespace cave { struct FrameData; }
namespace cave { struct UIFrameDrawData; }
// clang-format on

namespace cave::render {

class Renderer : public IService {
    class Impl;

public:
    Renderer();
    ~Renderer();

    void tick(const FrameTime& frame,
              std::span<const ResolvedView> views,
              const UIFrameDrawData& ui_data);

#if USING(USE_COMMAND)
    bool Cmd_dump(CommandContext& ctx, const CommandArgs& args);
#endif

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    std::unique_ptr<Impl> m_impl;
};

}  // namespace cave::render
