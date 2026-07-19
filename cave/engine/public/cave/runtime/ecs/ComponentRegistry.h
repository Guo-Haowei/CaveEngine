// =============================================================================
// File: cave/runtime/ecs/ComponentRegistry.h
// =============================================================================
#pragma once
#include <span>
#include <string_view>

#include "cave/core/containers/Containers.h"
#include "cave/core/ids/Entity.h"
#include "cave/core/reflection/Reflection.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {
class Scene;
}  // namespace cave

namespace cave::ecs {

struct ComponentMeta {
    ComponentId cid;
    const char* name;
    uint32_t size;
    uint32_t align;
    uint64_t version;

    std::span<const FieldMetaBase* const> props;

    const FieldMetaBase* find(const PropertyId& pid) const;
};

class ComponentRegistry {
public:
    void registerMeta(const ComponentMeta& meta);
    const ComponentMeta* tryGet(ComponentId pid) const;

    // For mutating on_edited only
    ComponentMeta& getMut(ComponentId pid);

    static void builtin(ComponentRegistry& out);

    static ComponentRegistry builtin() {
        ComponentRegistry reg;
        builtin(reg);
        return reg;
    }

private:
    Vector<ComponentMeta> m_table;
    HashMap<StringId, size_t> m_lookup;
};

}  // namespace cave::ecs
