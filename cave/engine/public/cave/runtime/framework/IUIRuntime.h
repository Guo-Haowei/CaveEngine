// =============================================================================
// File: cave/runtime/framework/IUIRuntime.h
// =============================================================================
#pragma once
#include <span>

#include "cave/core/containers/Containers.h"
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/ids/ViewId.h"
#include "cave/core/string/StringId.h"
#include "cave/ui/UIDrawCommand.h"
#include "cave/ui/UIInput.h"
#include "cave/runtime/ui/UITypes.h"

namespace cave {

class Scene;

struct UIButtonClicked {
    SceneId scene_id;
    StringId event;
    ecs::Entity source;
};

// @TODO: deprecate
struct UIFrameDrawData {
    HashMap<ViewId, UIDrawList> draw_lists;

    void clear() { draw_lists.clear(); }
};

class IUIRuntime {
public:
    virtual ~IUIRuntime() = default;

    virtual void beginFrame(const UIInput& input) = 0;
    virtual void endFrame() = 0;

    virtual std::span<const UIButtonClicked> events() const = 0;

    virtual UIFrameDrawData takeDrawData() = 0;
};

}  // namespace cave
