#include "cave/core/string/StringId.h"

namespace cave {

#if USING(STRING_ID_KEEP_SOURCE)
bool StringId::operator==(const StringId& rhs) const {
    if (rhs.hash_ != hash_) {
        return false;
    }

    if (rhs.debug_name_ != debug_name_) {
        LOG_FATAL("StringId:: hash '{}' collision detected - '{}' and '{}'",
                  hash_,
                  debug_name_.view(),
                  rhs.debug_name_.view());
        return false;
    }
    return true;
}
#endif

}  // namespace cave
