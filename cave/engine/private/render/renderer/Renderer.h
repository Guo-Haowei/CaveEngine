#pragma once
#include "cave/core/time/FrameTime.h"
#include "cave/runtime/framework/IService.h"

#include "engine/private/render/renderer/ResolvedView.h"

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

    void Tick(const FrameTime& p_frame,
              std::span<const ResolvedView> p_views,
              const UIFrameDrawData& p_ui_data);

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    std::unique_ptr<Impl> m_impl;
};

}  // namespace cave::render
