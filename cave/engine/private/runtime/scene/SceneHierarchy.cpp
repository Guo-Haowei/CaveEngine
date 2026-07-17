#include "SceneHierarchy.h"

#include "cave/runtime/ecs/components/HierarchyComponent.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using ::cave::ecs::Entity;

void SceneHierarchy::rebuild(const Scene& scene) {
    clear();

    for (const auto& [child, hier] : scene.view<HierarchyComponent>()) {
        addChild(hier.parent(), child);
    }
}

void SceneHierarchy::beginBulkEdit() {
    ++m_edit_depth;
}

void SceneHierarchy::endBulkEdit(const Scene& scene) {
    DEV_ASSERT(m_edit_depth > 0);

    if (--m_edit_depth == 0 && m_dirty) {
        rebuild(scene);
        m_dirty = false;
    }
}

void SceneHierarchy::clear() {
    m_roots.clear();
    m_children.clear();
}

bool SceneHierarchy::onParentChanged(Entity child,
                                     Entity old_parent,
                                     Entity new_parent) {
    if (old_parent == new_parent) {
        return false;
    }

    if (m_edit_depth > 0) {
        m_dirty = true;
        return true;
    }

    if (old_parent.valid()) {
        removeChild(old_parent, child);
    } else {
        auto it = std::find(m_roots.begin(), m_roots.end(), child);
        if (DEV_VERIFY(it != m_roots.end())) {
            m_roots.erase(it);
        }
    }

    addChild(new_parent, child);
    return true;
}

std::span<const Entity> SceneHierarchy::children(Entity parent) const {
    auto it = m_children.find(parent);
    if (it == m_children.end()) {
        return {};
    }

    return it->second;
}

void SceneHierarchy::addChild(Entity parent, Entity child) {
    if (parent.valid()) {
        auto& children = m_children[parent];

        DEV_ASSERT(std::find(children.begin(),
                             children.end(),
                             child) == children.end());

        children.push_back(child);
    } else {
        auto it = std::find(m_roots.begin(), m_roots.end(), child);

        if (DEV_VERIFY(it == m_roots.end())) {
            m_roots.push_back(child);
        }
    }
}

void SceneHierarchy::removeChild(Entity parent, Entity child) {
    auto it = m_children.find(parent);
    if (it == m_children.end()) {
        return;
    }

    const size_t old_size = it->second.size();
    std::erase(it->second, child);
    DEV_ASSERT(it->second.size() + 1 == old_size);

    if (it->second.empty()) {
        m_children.erase(it);
    }
}

Option<Entity> SceneHierarchy::firstRoot() const {
    if (m_roots.empty()) {
        return None();
    }
    return Some(m_roots[0]);
}

}  // namespace cave
