#pragma once
#include "cave/render/IViewProvider.h"

#include "engine/private/runtime/framework/Module.h"

namespace cave {

using ViewportId = uint32_t;
using ViewProviderRef = std::shared_ptr<render::IViewProvider>;

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

    void ClearViewport();

    void BuildViews(std::vector<render::ViewDesc>& p_out_views);

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

private:
    ViewportId m_next_id = 0;
    std::vector<ManagedViewport> m_viewports;
};

}  // namespace cave
