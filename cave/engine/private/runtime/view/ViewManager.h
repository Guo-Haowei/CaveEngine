#pragma once
#include "cave/core/ids/GenIdRegistry.h"
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/view/ViewDesc.h"
#include "cave/runtime/view/ViewRecord.h"

#include "ResolvedView.h"

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
        return Base::resolve(view_id);
    }

    const ViewRecord* resolve(ViewId view_id) const {
        return Base::resolve(view_id);
    }

private:
    SceneRegistry& m_scene_reg;
    const bool m_is_opengl;

    std::vector<ViewDesc> m_view_descs;
    std::vector<ResolvedView> m_resolved_views;
    bool m_can_submit{ false };
};

}  // namespace cave
