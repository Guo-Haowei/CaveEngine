#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/ui/ResolvedUI.h"

namespace cave {

class Scene;

struct UIRectTransformComponent;

class UILayoutResolver {
    using TreeLookup = HashMap<ecs::Entity, Vector<ecs::Entity>>;

public:
    ResolvedUICanvas resolve(const Scene& scene,
                             ecs::Entity canvas,
                             math::Vec2f canvas_size) const;

private:
    void resolveNode(const Scene& scene,
                     const TreeLookup& lookup,
                     ecs::Entity ent,
                     const UIRect& parent_rect,
                     ResolvedUICanvas& out) const;

    static UIRect resolveRect(const UIRect& parent_rect,
                              const UIRectTransformComponent& local);
};

}  // namespace cave
