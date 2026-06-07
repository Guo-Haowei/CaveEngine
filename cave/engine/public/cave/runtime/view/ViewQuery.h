// =============================================================================
// File: cave/runtime/view/ViewQuery.h
// =============================================================================
#pragma once
#include "cave/core/Option.h"
#include "cave/runtime/view/ViewRecord.h"

namespace cave {

class ViewManager;

class ViewQuery {
public:
    ViewQuery(ViewManager& view) noexcept
        : view_(view) {}

    const ViewRecord* resolve(ViewId view_id) const;

private:
    const ViewManager& view_;
};

}  // namespace cave
