#pragma once
#include "engine/runtime/module.h"
#include "engine/runtime/scene_view.h"

namespace cave {

class ISceneViewProvider;
struct SceneView;

using ViewportId = uint32_t;
using ViewProviderRef = std::shared_ptr<ISceneViewProvider>;

struct ManagedViewport {
    ViewportId id = 0;
    bool visible = true;
    bool focused = false;

    // FIntRect ViewRect{};

    // editor view, game view, preview view, etc.
    ViewProviderRef view_provider;
};

class ViewportManager : public Module {
public:
    ViewportManager();

    ViewportId CreateViewport(ViewProviderRef p_provider);

    void UpdateProviders(float p_timestep);

    void BuildViews(std::vector<SceneView>& p_out_views,
                    bool p_is_opengl);

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

private:
    ViewportId m_next_id = 0;
    std::vector<ManagedViewport> m_viewports;
};

}  // namespace cave
