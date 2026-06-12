#include "cave/core/error/ErrorCode.h"

#include <iterator>

namespace cave {

const char* ErrorToString(ErrorCode error) {
    static const char* s_errorNames[] = {
#define ERROR_CODE(NAME) #NAME,
        ERROR_LIST
#undef ERROR_CODE
    };

    static_assert(std::size(s_errorNames) == std::to_underlying(ErrorCode::COUNT));

    return s_errorNames[std::to_underlying(error)];
}

}  // namespace cave
