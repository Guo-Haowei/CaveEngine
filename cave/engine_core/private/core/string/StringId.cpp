#include "cave/core/diagnostics/Log.h"
#include "cave/core/string/StringId.h"

namespace cave {

#if USING(STRING_ID_KEEP_SOURCE)
bool StringId::operator==(const StringId& rhs) const {
    if (rhs.m_hash != m_hash) {
        return false;
    }

    if (rhs.m_debug_name != m_debug_name) {
        LOG_FATAL("StringId:: hash '{}' collision detected - '{}' and '{}'",
                  m_hash,
                  m_debug_name.view(),
                  rhs.m_debug_name.view());
        return false;
    }
    return true;
}
#endif

}  // namespace cave
