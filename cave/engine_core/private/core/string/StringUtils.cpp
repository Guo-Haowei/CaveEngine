#include "cave/core/string/StringUtils.h"

namespace cave {

bool StringUtils::equal(const char* str1, const char* str2) {
    str1 = str1 ? str1 : "";
    str2 = str2 ? str2 : "";
    return strcmp(str1, str2) == 0;
}

void StringUtils::replaceFirst(std::string& str,
                               std::string_view pattern,
                               std::string_view replace) {
    size_t pos = str.find(pattern);
    if (pos != std::string::npos) {
        str.replace(pos, pattern.size(), replace);
    }
}

char* StringUtils::strdup(const char* source) {
#if USING(PLATFORM_WINDOWS)
    return _strdup(source);
#elif USING(PLATFORM_APPLE) || USING(PLATFORM_WASM)
    return strdup(source);
#else
#error Platform not supported
#endif
}

void StringUtils::strcpy(char* dst, size_t dst_len, const char* src, size_t src_len) {
    if (!dst || dst_len == 0) return;

    if (!src || src_len == 0) {
        dst[0] = '\0';
        return;
    }

    const size_t copy_len = (dst_len - 1 < src_len) ? (dst_len - 1) : src_len;
    std::memcpy(dst, src, copy_len);
    dst[copy_len] = '\0';
    return;
}

std::string_view StringUtils::removeExtension(std::string_view sv) {
    size_t dot_pos = sv.find_last_of('.');
    return dot_pos == std::string_view::npos ? sv : sv.substr(0, dot_pos);
}

std::vector<std::string_view> StringUtils::tokenize(std::string_view sv) {
    std::vector<std::string_view> out;

    size_t i = 0;
    while (i < sv.size()) {
        // skip spaces
        while (i < sv.size() && std::isspace((unsigned char)sv[i]))
            ++i;

        if (i >= sv.size())
            break;

        size_t start = i;

        // read token
        while (i < sv.size() && !std::isspace((unsigned char)sv[i]))
            ++i;

        out.emplace_back(sv.data() + start, i - start);
    }

    return out;
}

}  // namespace cave
