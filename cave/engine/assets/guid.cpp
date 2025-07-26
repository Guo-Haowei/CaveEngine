#include "guid.h"

#if USING(PLATFORM_WINDOWS)
#include <objbase.h>
#endif

#include "engine/core/string/string_builder.h"
#include "engine/core/string/string_utils.h"

namespace cave {

Guid Guid::Create() {
    Guid result;
#if USING(PLATFORM_WINDOWS)
    static_assert(sizeof(Guid) == sizeof(GUID));
    GUID guid;

    ::CoCreateGuid(&guid);
    memcpy(&result, &guid, sizeof(Guid));
#else
    CRASH_NOW_MSG("DON'T CALL THIS");
#endif
    return result;
}

Option<Guid> Guid::Parse(const char* p_start, size_t p_length) {
    if (p_length != 35 /* 16 x 2 + 3 */) {
        return None();
    }

    Guid guid;
    int i = 0;
    int buffer_index = 0;
    do {
        char c = p_start[i];
        if (buffer_index == 4 || buffer_index == 6 || buffer_index == 8) {
            if (c != '-') {
                return None();
            }

            ++i;  // skip '-'
        }

        const char high = StringUtils::HexToInt(p_start[i]);
        const char low = StringUtils::HexToInt(p_start[i + 1]);
        if (low < 0 || high < 0) {
            return None();
        }

        guid.m_data[buffer_index++] = (high << 4) | (low);
        i += 2;
    } while (i < p_length);

    return Some(guid);
}

std::string Guid::ToString() const {
    char buf[64];
    const uint8_t* data = m_data;
    std::snprintf(buf, sizeof(buf),
                  "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X%02X%02X",
                  data[0], data[1], data[2], data[3],
                  data[4], data[5], data[6], data[7],
                  data[8], data[9], data[10], data[11],
                  data[12], data[13], data[14], data[15]);
    return buf;
}

}  // namespace cave
