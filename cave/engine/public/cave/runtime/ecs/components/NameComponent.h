// =============================================================================
// File: cave/runtime/ecs/components/NameComponent.h
// =============================================================================
#pragma once
#include "cave/core/containers/FixedString.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class NameComponent {
    CAVE_COMPONENT(NameComponent)

private:
    CAVE_PROP()
    FixedString<64> m_name;

public:
    NameComponent() = default;

    NameComponent(const char* p_name) { m_name = p_name; }

    void SetName(const char* p_name) { m_name = p_name; }
    void SetName(std::string_view p_name) { m_name = p_name; }

    std::string_view GetName() const { return m_name.view(); }
    FixedString<64>& GetNameRef() { return m_name; }
};

}  // namespace cave
