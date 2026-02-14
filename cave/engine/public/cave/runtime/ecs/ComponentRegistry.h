// =============================================================================
// File: engine/public/cave/runtime/ecs/ComponentRegistry.h
// =============================================================================
#pragma once
#include <span>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "cave/core/reflection/Reflection.h"
#include "cave/runtime/ecs/ComponentDefines.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave {
using PropertyId = std::string_view;

class Scene;
}  // namespace cave

namespace cave::ecs {

using OnComponentEditedFn = void (*)(Scene&, ecs::Entity, ComponentId, PropertyId);

struct ComponentMeta {
    ComponentId id;
    const char* name;
    uint32_t size;
    uint32_t align;
    uint64_t version;

    std::span<const FieldMetaBase* const> props;
    OnComponentEditedFn on_edited{ nullptr };

    const FieldMetaBase* Find(PropertyId p_id) const;
};

class ComponentRegistry {
public:
    void Register(const ComponentMeta& p_meta);
    const ComponentMeta* TryGet(ComponentId p_id) const;

    // For mutating on_edited only
    ComponentMeta& GetMut(ComponentId p_id);

private:
    std::vector<ComponentMeta> m_table;
    std::vector<uint8_t> m_present;
};

}  // namespace cave::ecs
