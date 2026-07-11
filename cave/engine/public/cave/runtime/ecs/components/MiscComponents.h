// =============================================================================
// File: cave/runtime/ecs/components/MiscComponents.h
// =============================================================================
#pragma once
#include "cave/core/containers/FixedString.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

struct PrefabChildComponent {
    CAVE_COMPONENT(PrefabChildComponent)
};

struct PendingDestroyComponent {
    CAVE_COMPONENT(PendingDestroyComponent)
};

class NameComponent {
    CAVE_COMPONENT(NameComponent)

private:
    CAVE_PROP()
    FixedString<64> m_name;

public:
    NameComponent() = default;

    NameComponent(const char* name) { m_name = name; }

    void setName(const char* name) { m_name = name; }
    void setName(std::string_view name) { m_name = name; }

    std::string_view name() const { return m_name.view(); }
    FixedString<64>& nameRef() { return m_name; }

    bool operator==(const NameComponent& rhs) const {
        return m_name.view() == rhs.m_name.view();
    }

    bool operator!=(const NameComponent& rhs) const {
        return m_name.view() != rhs.m_name.view();
    }
};

}  // namespace cave
