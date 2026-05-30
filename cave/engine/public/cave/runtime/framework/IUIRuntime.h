// =============================================================================
// File: engine/public/cave/runtime/framework/IUIRuntime.h
// =============================================================================
#pragma once
#include <unordered_map>
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/framework/IService.h"
#include "cave/ui/UIDrawCommand.h"
#include "cave/ui/UIInput.h"
#include "cave/ui/UITypes.h"

namespace cave {

struct UIFrameDrawData {
    std::unordered_map<ViewId, UIDrawList> draw_lists;

    void Clear() { draw_lists.clear(); }
};

class IUIRuntime : public IService {
public:
    using IService::IService;

    virtual void BeginFrame(const UIInput& p_input) = 0;
    virtual void EndFrame() = 0;

    virtual void BeginView(ViewId p_view_id) = 0;
    virtual void EndView() = 0;

    virtual bool Button(UIId p_id, UIRect p_rect) = 0;

    virtual UIFrameDrawData TakeDrawData() = 0;
};

}  // namespace cave
