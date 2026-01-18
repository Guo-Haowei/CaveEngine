#include "viewport_manager.h"

#include "engine/runtime/scene_view_provider_interface.h"

namespace cave {

ViewportManager::ViewportManager()
    : Module("ViewportManager") {}

auto ViewportManager::InitializeImpl() -> Result<void> {
    return Result<void>();
}

void ViewportManager::FinalizeImpl() {
}

ViewportId ViewportManager::CreateViewport(ViewProviderRef p_provider, const char* p_debug_name) {
    ManagedViewport V;
    V.id = ++m_next_id;
    V.debug_name = p_debug_name ? p_debug_name : "Viewport";
    V.view_provider = std::move(p_provider);

    m_viewports.push_back(std::move(V));
    return m_viewports.back().id;
}

void ViewportManager::BuildViews(float p_timestep, std::vector<SceneView>& p_out_views) {
    p_out_views.clear();

    for (auto& vp : m_viewports) {
        if (!vp.visible || !vp.view_provider) {
            continue;
        }

        vp.view_provider->Update(p_timestep, vp.focused);
        vp.view_provider->BuildViews(p_out_views);
    }
}

}  // namespace cave
