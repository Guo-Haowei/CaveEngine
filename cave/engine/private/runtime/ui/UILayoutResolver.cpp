#include "UILayoutResolver.h"

#include "cave/runtime/display/ICanvas.h"
#include "cave/runtime/ui/UIComponents.h"
#include "cave/runtime/ecs/components/HierarchyComponent.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using namespace ::cave::math;
using ::cave::ecs::Entity;

ResolvedUITree UILayoutResolver::resolve(const Scene& scene,
                                         Entity canvas,
                                         math::Vec2f canvas_size) const {
    DEV_ASSERT(scene.count<UIRectTransformComponent>());

    HashMap<Entity, Vector<Entity>> tree_lookup;
    for (auto [ent, rect, hier] : scene.view<UIRectTransformComponent, HierarchyComponent>()) {
        DEV_ASSERT(hier.parent_id.valid());
        tree_lookup[hier.parent_id].push_back(ent);
    }

    UIRect canvas_rect{ Vec2f::Zero, canvas_size };

    ResolvedUITree tree;
    auto it = tree_lookup.find(canvas);
    DEV_ASSERT(it != tree_lookup.end());
    for (Entity e : it->second) {
        resolveNode(scene, tree_lookup, e, canvas_rect, tree);
    }

    return tree;
}

void UILayoutResolver::resolveNode(const Scene& scene,
                                   const TreeLookup& lookup,
                                   Entity ent,
                                   const UIRect& parent_rect,
                                   ResolvedUITree& out) const {
    auto* hierarchy = scene.component<HierarchyComponent>(ent);
    DEV_ASSERT(hierarchy);

    auto* transform = scene.component<UIRectTransformComponent>(ent);
    DEV_ASSERT(transform);
    if (!transform) {
        return;
    }

    const UIRect resolved_rect = resolveRect(parent_rect, *transform);

    out.elements.push_back(ResolvedUIElement{
        .entity = ent,
        .rect = resolved_rect,
        .effective_visible = hierarchy->visible,
        .draw_order = static_cast<uint32_t>(out.elements.size()),
    });

    auto it = lookup.find(ent);
    if (it == lookup.end()) return;
    for (Entity child : it->second) {
        resolveNode(scene,
                    lookup,
                    child,
                    resolved_rect,
                    out);
    }
}

UIRect UILayoutResolver::resolveRect(const UIRect& parent_rect,
                                     const UIRectTransformComponent& local) {

    const Vec2f parent_size = parent_rect.max() - parent_rect.min();

    const Vec2f resolved_anchor_min = parent_rect.min() +
                                      parent_size * local.anchor_min;

    const Vec2f resolved_anchor_max = parent_rect.min() +
                                      parent_size * local.anchor_max;

    return UIRect{
        resolved_anchor_min + local.offset_min,
        resolved_anchor_max + local.offset_max,
    };
}

}  // namespace cave
