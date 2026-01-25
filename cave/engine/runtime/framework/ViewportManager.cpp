#include "ViewportManager.h"

#include "engine/runtime/framework/Application.h"

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

void ViewportManager::ClearViewport() {
    m_next_id = 0;
    m_viewports.clear();
}

void ViewportManager::BuildViews(std::vector<SceneView>& p_out_views,
                                 bool p_is_opengl) {
    p_out_views.clear();
    for (auto& vp : m_viewports) {
        vp.view_provider->BuildViews(p_out_views, p_is_opengl);
    }
}

}  // namespace cave
