#pragma once
#include "cave/core/variant/Variant.h"

#include "Dvar.h"

namespace cave {

// @TODO: move
struct TransparentStringHash {
    using is_transparent = void;

    size_t operator()(std::string_view value) const noexcept {
        return std::hash<std::string_view>{}(value);
    }

    size_t operator()(const std::string& value) const noexcept {
        return operator()(std::string_view{ value });
    }

    size_t operator()(const char* value) const noexcept {
        return operator()(std::string_view{ value });
    }
};

template<typename T>
using StringHashMap = std::unordered_map<
    std::string,
    T,
    TransparentStringHash,
    std::equal_to<>>;

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

private:
    bool registerImpl(const String& name, Entry&& entry);

    Vector<Entry> m_storage;
    StringHashMap<uint32_t> m_entry_lookup;
};

}  // namespace cave