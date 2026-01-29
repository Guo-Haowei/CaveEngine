// =============================================================================
// File: public/cave/render/IViewProvider.h
// =============================================================================
#pragma once
#include <vector>
#include "cave/core/ids/DebugId.h"
#include "cave/render/ViewDesc.h"

namespace cave::render {

class IViewProvider {
public:
    virtual ~IViewProvider() = default;

    virtual void BuildViews(std::vector<ViewDesc>& p_out_views) = 0;

    virtual DebugId GetDebugId() = 0;
};

}  // namespace cave::render