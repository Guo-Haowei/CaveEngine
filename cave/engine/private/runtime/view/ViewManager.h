#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/view/ViewDesc.h"
#include "cave/runtime/view/ViewRecord.h"

#include "ResolvedView.h"

#include "engine/private/core/ids/GenIdRegistry.h"

namespace cave {

class SceneRegistry;

class ViewManager : protected GenIdRegistry<ViewRecord> {
    using Base = GenIdRegistry<ViewRecord>;

public:
    ViewManager(SceneRegistry& scene_reg, bool is_opengl) noexcept;

    void beginFrame();
    std::span<const ResolvedView> endFrame();

    ViewId createView(std::string_view debug_name,
                      const math::IntRect& viewport_px);
    void destroyView(ViewId view_id);

    void submit(const ViewDesc& view_desc);

    ViewRecord* resolve(ViewId view_id) {
        return Base::Resolve(view_id);
    }

    const ViewRecord* resolve(ViewId view_id) const {
        return Base::Resolve(view_id);
    }

private:
    SceneRegistry& scene_reg_;
    const bool is_opengl_;

    std::vector<ViewDesc> view_descs_;
    std::vector<ResolvedView> resolved_views_;
    bool can_submit_{ false };
};

}  // namespace cave
