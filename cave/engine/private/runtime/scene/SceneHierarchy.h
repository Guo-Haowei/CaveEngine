#pragma once
#include "cave/core/containers/Containers.h"
#include "cave/core/ids/Entity.h"

namespace cave {

class Scene;

class SceneHierarchy {
public:
    void rebuild(const Scene& scene);
    void clear();

    void beginBulkEdit();
    void endBulkEdit(const Scene& scene);

    bool onParentChanged(ecs::Entity child,
                         ecs::Entity old_parent,
                         ecs::Entity new_parent);

    std::span<const ecs::Entity> children(ecs::Entity parent) const;
    std::span<const ecs::Entity> roots() const { return m_roots; }

private:
    void addChild(ecs::Entity parent, ecs::Entity child);
    void removeChild(ecs::Entity parent, ecs::Entity child);

    HashMap<ecs::Entity, Vector<ecs::Entity>> m_children;
    Vector<ecs::Entity> m_roots;

    int m_edit_depth = 0;
    bool m_dirty = true;
};

}  // namespace cave
