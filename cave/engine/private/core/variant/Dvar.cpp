#include "Dvar.h"

#if USING(ENABLE_DVAR)
#include "DvarTable.h"

namespace cave {

Dvar* FindStaticDvar(std::string_view name) {
    return DvarTable::global().find(name);
}

bool RegisterStaticDvar(Dvar* dvar) {
    return DvarTable::global().registerStatic(dvar);
}

Dvar::Dvar(String name,
           Variant&& variant,
           DvarFlags flags,
           const char* desc)
    : m_name(std::move(name))
    , m_variant(std::move(variant))
    , m_desc(desc)
    , m_flags(flags) {
}

bool Dvar::setValue(Variant&& variant) {
    if (type() != variant.type()) {
        LOG_ERROR(LogChannel::Dvar, "{} type mismatch", name());
        return false;
    }

    m_variant = std::move(variant);
    return true;
}

}  // namespace cave
#endif
