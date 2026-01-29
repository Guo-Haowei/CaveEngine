#pragma once
#include "cave/render/ViewDesc.h"
#include "engine/private/runtime/framework/Module.h"

// clang-format off
namespace cave::render { class RenderSystemImpl; }
// clang-format on

namespace cave {

class CameraComponent;
class Scene;

struct FrameData;

class RenderSystem : public Module {
public:
    RenderSystem();
    ~RenderSystem();

    void BeginFrame();

    void RenderFrame(const std::vector<render::ViewDesc>& p_views);

    const FrameData* GetFrameData() const;

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    FrameData* m_frameData{ nullptr };
    std::unique_ptr<render::RenderSystemImpl> m_impl;
};

}  // namespace cave
