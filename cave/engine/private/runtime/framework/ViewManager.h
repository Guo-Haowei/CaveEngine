#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/render/ViewDesc.h"
#include "cave/runtime/framework/IService.h"

#include "engine/private/core/ids/GenIdRegistry.h"
#include "engine/private/render/renderer/ResolvedView.h"

namespace cave {

class ViewManager : public IService,
                    public GenIdRegistry<internal::View> {
public:
    ViewManager();

    void BeginFrame();
    std::span<const render::ResolvedView> EndFrame();

    void Submit(const render::ViewDesc& p_view_desc);

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

private:
    std::vector<render::ViewDesc> m_view_descs;
    std::vector<render::ResolvedView> m_views;
    bool m_can_submit{ false };
};

}  // namespace cave
