#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/view/ViewDesc.h"
#include "cave/runtime/framework/IService.h"

#include "ResolvedView.h"
#include "ViewRecord.h"

#include "engine/private/core/ids/GenIdRegistry.h"

namespace cave {

class ViewManager : public IService,
                    protected GenIdRegistry<ViewRecord> {
    using Base = GenIdRegistry<ViewRecord>;

public:
    ViewManager();

    void BeginFrame();
    std::span<const ResolvedView> EndFrame();

    ViewId CreateView(std::string_view p_debug_name,
                      const math::IntRect& p_viewport_px);
    void DestroyView(ViewId p_view_id);

    void Submit(const ViewDesc& p_view_desc);
    ViewRecord* Resolve(ViewId p_view_id) {
        return Base::Resolve(p_view_id);
    }

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

private:
    std::vector<ViewDesc> m_view_descs;
    std::vector<ResolvedView> m_views;
    bool m_can_submit{ false };
};

}  // namespace cave
