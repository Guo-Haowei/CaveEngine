#pragma once
#include "engine/runtime/module.h"

namespace cave {

class CameraComponent;
struct FrameData;
class Scene;

class RenderSystem : public Module {
public:
    RenderSystem()
        : Module("RenderSystem") {}

    void BeginFrame();

    void RenderFrame(Scene* p_scene);

    const FrameData* GetFrameData() const { return m_frameData; }

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    FrameData* m_frameData{ nullptr };
};

}  // namespace cave
