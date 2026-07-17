#include "SceneHierarchy.h"

namespace cave {

using ::cave::ecs::Entity;

void SceneHierarchy::rebuild(const Scene& scene) {
    clear();
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

    if (old_parent.valid()) {
        removeChild(old_parent, child);
    } else {
        const size_t old_size = m_roots.size();
        std::erase(m_roots, child);
        DEV_ASSERT(m_roots.size() + 1 == old_size);
    }

    if (new_parent.valid()) {
        addChild(new_parent, child);
    } else {
        if (std::find(m_roots.begin(),
                      m_roots.end(),
                      new_parent) == m_roots.end()) {
            m_roots.push_back(child);
        }
    }
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
    auto& children = m_children[parent];

    DEV_ASSERT(std::find(children.begin(),
                         children.end(),
                         child) == children.end());

    children.push_back(child);
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

}  // namespace cave
