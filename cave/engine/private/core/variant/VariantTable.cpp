#include "VariantTable.h"

namespace cave {

Dvar* DvarTable::find(std::string_view sv) {
    auto it = m_entry_lookup.find(sv);
    if (it == m_entry_lookup.end()) {
        return nullptr;
    }

    if (DEV_VERIFY(it->second < m_storage.size())) {
        return m_storage[it->second].get();
    }

    return nullptr;
}

bool DvarTable::registerStatic(Dvar* dvar) {
    DEV_ASSERT(dvar);

    return registerImpl(dvar->name(),
                        {
                            .external = dvar,
                            .owned = nullptr,
                        });
}

bool DvarTable::registerDynamic(Owner<Dvar>&& dvar) {
    DEV_ASSERT(dvar);

    String name = dvar->name();
    return registerImpl(name,
                        {
                            .external = nullptr,
                            .owned = std::move(dvar),
                        });
}

bool DvarTable::registerImpl(const String& name, Entry&& entry) {
    const uint32_t slot = static_cast<uint32_t>(m_storage.size());
    auto [it, ok] = m_entry_lookup.try_emplace(name, slot);
    if (!ok) {
        LOG_ERROR(LogChannel::Dvar, "dvar {} already registered!", name);
        return false;
    }

    m_storage.push_back(std::move(entry));
    return true;
}

}  // namespace cave
