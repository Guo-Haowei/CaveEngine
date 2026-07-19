// =============================================================================
// File: cave/core/reflection/MetaRegistry.h
// =============================================================================
#pragma once
#include <span>
#include <string_view>

#include "cave/core/containers/Containers.h"
#include "cave/core/reflection/Reflection.h"

namespace cave {

struct MetaTable {
    StringId type_id;
    const char* name;
    uint32_t size;
    uint32_t align;
    uint64_t version;

    std::span<const FieldMetaBase* const> props;

    const FieldMetaBase* find(const PropertyId& pid) const;
};

class MetaRegistry {
public:
    void registerMeta(const MetaTable& meta);
    const MetaTable* tryGet(StringId pid) const;

    static void builtin(MetaRegistry& out);

    // only used for debug
    static MetaRegistry builtin() {
        MetaRegistry reg;
        builtin(reg);
        return reg;
    }

private:
    Vector<MetaTable> m_table;
    HashMap<StringId, size_t> m_lookup;
};

}  // namespace cave
