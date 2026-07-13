#pragma once
#include "cave/core/containers/StringHash.h"
#include "cave/core/variant/Variant.h"

#include "Dvar.h"

namespace cave {

class DvarTable {
    struct Entry {
        Dvar* external = nullptr;
        Owner<Dvar> owned;

        Dvar* get() noexcept {
            DEV_ASSERT((external != nullptr) != (owned != nullptr));
            return external ? external : owned.get();
        }

        const Dvar* get() const noexcept {
            DEV_ASSERT((external != nullptr) != (owned != nullptr));
            return external ? external : owned.get();
        }
    };

public:
    Dvar* find(std::string_view sv);
    bool registerStatic(Dvar* dvar);
    bool registerDynamic(Owner<Dvar>&& dvar);

    void serialize(std::string_view path);
    void deserialize(std::string_view path);
    bool parse(std::span<std::string_view> commands);

    static DvarTable& global();

private:
    bool registerImpl(const String& name, Entry&& entry);

    Vector<Entry> m_storage;
    StringHashMap<uint32_t> m_entry_lookup;
};

}  // namespace cave