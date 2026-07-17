// =============================================================================
// File: cave/runtime/ecs/components/HierarchyComponent.h
// =============================================================================
#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

struct FieldChange;

class HierarchyComponent {
    CAVE_COMPONENT(HierarchyComponent)

private:
    CAVE_PROP(on_change = onParentChanged)
    ecs::Entity m_parent_id;

    CAVE_PROP()
    bool m_local_visible = true;

    // Non-serialized attributes
    bool m_visible = true;

    void onParentChanged(const FieldChange& change);

public:
    ecs::Entity parent() const { return m_parent_id; }
    void setParent(ecs::Entity parent) { m_parent_id = parent; }

    bool localVisible() const { return m_local_visible; }
    void setLocalVisible(bool value) { m_local_visible = value; }

    bool visible() const { return m_visible; }
    void setVisible(bool value) { m_visible = value; }
};

}  // namespace cave
