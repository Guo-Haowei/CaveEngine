// =============================================================================
// File: public/cave/runtime/view/ViewQuery.h
// =============================================================================
#pragma once
#include "cave/core/Option.h"
#include "cave/runtime/view/ViewRecord.h"

namespace cave {

class ViewManager;

class ViewQuery {
public:
    ViewQuery(ViewManager& p_view) noexcept
        : m_view(p_view) {}

    const ViewRecord* Resolve(ViewId p_view_id) const;

private:
    const ViewManager& m_view;
};

}  // namespace cave
