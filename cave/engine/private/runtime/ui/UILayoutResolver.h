#pragma once
#include "cave/core/containers/Containers.h"
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/ui/UITypes.h"

namespace cave {

class Scene;

struct UIRectTransformComponent;

struct ResolvedUIElement {
    ecs::Entity entity;

    UIRect rect;  // Absolute canvas-space pixels.

    bool effective_visible = true;
    uint32_t draw_order = 0;
};

struct ResolvedUITree {
    ecs::Entity canvas;
    math::Vec2f canvas_size;

    // Parent-before-children, matching draw order.
    Vector<ResolvedUIElement> elements;
};

class UILayoutResolver {
    using TreeLookup = HashMap<ecs::Entity, Vector<ecs::Entity>>;

public:
    ResolvedUITree resolve(const Scene& scene,
                           ecs::Entity canvas,
                           math::Vec2f canvas_size) const;

private:
    void resolveNode(const Scene& scene,
                     const TreeLookup& lookup,
                     ecs::Entity ent,
                     const UIRect& parent_rect,
                     ResolvedUITree& out) const;

    static UIRect resolveRect(const UIRect& parent_rect,
                              const UIRectTransformComponent& local);
};

}  // namespace cave
