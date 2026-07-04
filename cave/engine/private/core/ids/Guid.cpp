#include "cave/core/ids/Guid.h"

#if USING(PLATFORM_WINDOWS)
#include <objbase.h>
#endif

#include "cave/core/string/StringUtils.h"

namespace cave {

Guid Guid::make() {
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

Option<Guid> Guid::parse(const char* start, size_t length) {
    if (length != 35 /* 16 x 2 + 3 */) {
        return None();
    }

    Guid guid;
    int i = 0;
    int buffer_index = 0;
    do {
        char c = start[i];
        if (buffer_index == 4 || buffer_index == 6 || buffer_index == 8) {
            if (c != '-') {
                return None();
            }

            ++i;  // skip '-'
        }

        const char high = StringUtils::HexToInt(start[i]);
        const char low = StringUtils::HexToInt(start[i + 1]);
        if (low < 0 || high < 0) {
            return None();
        }

        guid.data_[buffer_index++] = (high << 4) | (low);
        i += 2;
    } while (i < length);

    return Some(guid);
}

std::string Guid::toString() const {
    char buf[64];
    const uint8_t* data = data_.data();
    std::snprintf(buf, sizeof(buf),
                  "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X%02X%02X",
                  data[0], data[1], data[2], data[3],
                  data[4], data[5], data[6], data[7],
                  data[8], data[9], data[10], data[11],
                  data[12], data[13], data[14], data[15]);
    return buf;
}

}  // namespace cave
