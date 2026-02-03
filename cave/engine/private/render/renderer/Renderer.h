#pragma once
#include "engine/private/render/renderer/ResolvedView.h"
#include "engine/private/runtime/framework/Module.h"

// clang-format off
namespace cave { struct FrameData; }
// clang-format on

namespace cave::render {

class Renderer : public Module {
    class Impl;

public:
    Renderer();
    ~Renderer();

    void Tick(std::span<const ResolvedView> p_views);

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    std::unique_ptr<Impl> m_impl;
};

}  // namespace cave::render
