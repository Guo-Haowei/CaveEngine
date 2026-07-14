// =============================================================================
// File: cave/runtime/ui/IUIRuntime.h
// =============================================================================
#pragma once
#include <span>

#include "cave/core/containers/Containers.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/ui/ResolvedUI.h"
#include "cave/runtime/ui/UIInput.h"
#include "cave/runtime/ui/UITypes.h"

namespace cave {

class Scene;

struct UIControlId {
    SceneId scene_id{};
    ecs::Entity entity{};

    bool operator==(const UIControlId&) const = default;
};

struct UIInteractionState {
    Option<UIControlId> hovered;
    Option<UIControlId> active;
};

class IUIRuntime {
public:
    virtual ~IUIRuntime() = default;

    virtual void beginFrame() = 0;
    virtual void endFrame(const UIInput& ui_input) = 0;

    virtual void resolve(const Scene& scene, SceneId scene_id) = 0;
    virtual const ResolvedUICanvas* findResolved(SceneId scene_id,
                                                 ecs::Entity canvas_entity) const = 0;

    virtual UIInteractionState& interactionState() = 0;
};

}  // namespace cave
