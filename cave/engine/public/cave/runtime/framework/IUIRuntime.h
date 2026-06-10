// =============================================================================
// File: cave/runtime/framework/IUIRuntime.h
// =============================================================================
#pragma once
#include <unordered_map>
#include "cave/core/ids/ViewId.h"
#include "cave/ui/UIDrawCommand.h"
#include "cave/ui/UIInput.h"
#include "cave/ui/UITypes.h"

namespace cave {

struct UIFrameDrawData {
    std::unordered_map<ViewId, UIDrawList> draw_lists;

    void clear() { draw_lists.clear(); }
};

class IUIRuntime {
public:
    virtual ~IUIRuntime() = default;

    virtual void beginFrame(const UIInput& input) = 0;
    virtual void endFrame() = 0;

    virtual void beginView(ViewId view_id) = 0;
    virtual void endView() = 0;

    virtual bool button(UIId id, UIRect rect) = 0;

    virtual UIFrameDrawData takeDrawData() = 0;
};

}  // namespace cave
