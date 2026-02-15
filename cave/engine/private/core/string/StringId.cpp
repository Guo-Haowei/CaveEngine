#include "cave/core/string/StringId.h"

namespace cave {

#if USING(STRING_ID_KEEKP_SOURCE)
bool StringId::operator==(const StringId& p_other) const {
    if (p_other.m_hash != m_hash) {
        return false;
    }

    if (p_other.m_debug != m_debug) {
        LOG_FATAL("StringId:: hash '{}' collision detected - '{}' and '{}'",
                  m_hash,
                  m_debug.view(),
                  p_other.m_debug.view());
        return false;
    }
    return true;
}
#endif

}  // namespace cave
