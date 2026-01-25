#pragma once
#include "engine/private/runtime/framework/Module.h"

namespace cave {

class CameraComponent;
class Scene;

struct FrameData;
struct SceneView;

class RenderSystem : public Module {
public:
    RenderSystem()
        : Module("RenderSystem") {}

    void BeginFrame();

    void RenderFrame(std::vector<SceneView>& p_views);

    const FrameData* GetFrameData() const { return m_frameData; }

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    FrameData* m_frameData{ nullptr };
};

}  // namespace cave
