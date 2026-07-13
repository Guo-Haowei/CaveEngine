// =============================================================================
// File: cave/runtime/ui/IUIRuntime.h
// =============================================================================
#pragma once
#include <span>

#include "cave/core/containers/Containers.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/ui/ResolvedUI.h"
#include "cave/runtime/ui/UITypes.h"

// @TODO: deprecate
#include "cave/ui/UIDrawCommand.h"

namespace cave {

class Scene;

// @TODO: deprecate
struct UIFrameDrawData {
    HashMap<ViewId, UIDrawList> draw_lists;

    void clear() { draw_lists.clear(); }
};

class IUIRuntime {
public:
    virtual ~IUIRuntime() = default;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    virtual void resolve(const Scene& scene, SceneId scene_id) = 0;
    virtual const ResolvedUICanvas* findResolved(SceneId scene_id,
                                                 ecs::Entity canvas_entity) const = 0;

    virtual UIFrameDrawData takeDrawData() = 0;
};

}  // namespace cave
