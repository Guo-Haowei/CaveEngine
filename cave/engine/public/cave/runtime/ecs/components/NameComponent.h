// =============================================================================
// File: engine/public/cave/runtime/ecs/components/NameComponent.h
// =============================================================================
#pragma once
#include "cave/core/reflection/Reflection.h"
#include "cave/core/string/FixedString.h"

namespace cave {

class NameComponent {
    CAVE_META(NameComponent)

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
