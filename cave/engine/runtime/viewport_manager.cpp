#include "viewport_manager.h"

#include "engine/runtime/application.h"
#include "engine/runtime/input_manager.h"

namespace cave {

ViewportManager::ViewportManager()
    : Module("ViewportManager") {}

auto ViewportManager::InitializeImpl() -> Result<void> {
    return Result<void>();
}

void ViewportManager::FinalizeImpl() {
}

ViewportId ViewportManager::CreateViewport(ViewProviderRef p_provider) {
    ManagedViewport vp;
    vp.id = ++m_next_id;
    vp.view_provider = std::move(p_provider);

    m_viewports.push_back(std::move(vp));
    return m_viewports.back().id;
}

void ViewportManager::BuildViews(float p_timestep,
                                 std::vector<SceneView>& p_out_views,
                                 bool p_is_opengl) {
    p_out_views.clear();

    // @TODO: only build input for focused viewport
    ViewportInput input;
    m_app->GetInputManager()->FillViewportInput(input);

    for (auto& vp : m_viewports) {
        if (!vp.visible || !vp.view_provider) {
            continue;
        }

        vp.view_provider->Update(p_timestep, input, vp.focused);
        vp.view_provider->BuildViews(p_out_views, p_is_opengl);
    }
}

}  // namespace cave
