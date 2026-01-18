#pragma once
#include "engine/runtime/module.h"
#include "engine/runtime/scene_view_provider_interface.h"

namespace cave {

class ISceneViewProvider;
class Scene;

struct SceneView {
    ViewInfo view_info;
    Scene* scene{ nullptr };
};

using ViewportId = uint32_t;
using ViewProviderRef = std::unique_ptr<ISceneViewProvider>;

struct ManagedViewport {
    ViewportId id = 0;
    std::string debug_name;
    bool visible = true;
    bool focused = false;

    // FIntRect ViewRect{};

    // editor view, game view, preview view, etc.
    ViewProviderRef view_provider;
};

class ViewportManager : public Module {
public:
    ViewportManager();

    ViewportId CreateViewport(ViewProviderRef p_provider, const char* p_debug_name );
    void BuildViews(float dt, std::vector<SceneView>& outViews);

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

private:
    ViewportId m_next_id = 0;
    std::vector<ManagedViewport> m_viewports;
};

}  // namespace cave
