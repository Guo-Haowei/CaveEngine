#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/view/ViewDesc.h"
#include "cave/runtime/framework/IService.h"

#include "engine/private/core/ids/GenIdRegistry.h"
#include "engine/private/runtime/view/ResolvedView.h"

namespace cave {

class ViewManager : public IService,
                    protected GenIdRegistry<internal::View> {
    using Base = GenIdRegistry<internal::View>;

public:
    ViewManager();

    void BeginFrame();
    std::span<const ResolvedView> EndFrame();

    void Submit(const ViewDesc& p_view_desc);

    ViewId Create();
    void Destroy(ViewId p_view_id);

protected:
    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

private:
    std::vector<ViewDesc> m_view_descs;
    std::vector<ResolvedView> m_views;
    bool m_can_submit{ false };
};

}  // namespace cave
