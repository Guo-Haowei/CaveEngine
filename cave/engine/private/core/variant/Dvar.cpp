#include "Dvar.h"

#if USING(ENABLE_DVAR)
#include "VariantTable.h"

namespace cave {

using namespace cave::math;

static DvarTable s_table;

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

Dvar* FindStatic(std::string_view name) {
    return s_table.find(name);
}

bool RegisterStatic(Dvar* dvar) {
    return s_table.registerStatic(dvar);
}

}  // namespace cave
#endif
