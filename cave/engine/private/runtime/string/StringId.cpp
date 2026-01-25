#include "cave/runtime/string/StringId.h"

namespace cave {

#if USING(STRING_ID_KEEKP_SOURCE)
bool StringId::operator==(const StringId& p_other) const {
    if (p_other.m_hash != m_hash) {
        return false;
    }

    if (p_other.m_source != m_source) {
        LOG_FATAL("StringId:: hash '{}' collision detected - '{}' and '{}'",
                  m_hash,
                  m_source,
                  p_other.m_source);
        return false;
    }
    return true;
}
#endif

}  // namespace cave
