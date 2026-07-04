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
    FixedString<64> name_;

public:
    NameComponent() = default;

    NameComponent(const char* name) { name_ = name; }

    void setName(const char* name) { name_ = name; }
    void setName(std::string_view name) { name_ = name; }

    std::string_view name() const { return name_.view(); }
    FixedString<64>& nameRef() { return name_; }
};

}  // namespace cave
