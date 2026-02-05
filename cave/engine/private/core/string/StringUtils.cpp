#include "StringUtils.h"

namespace cave {

bool StringUtils::StringEqual(const char* p_str1, const char* p_str2) {
    p_str1 = p_str1 ? p_str1 : "";
    p_str2 = p_str2 ? p_str2 : "";
    return strcmp(p_str1, p_str2) == 0;
}

void StringUtils::ReplaceFirst(std::string& p_string, std::string_view p_pattern, std::string_view p_replacement) {
    size_t pos = p_string.find(p_pattern);
    if (pos != std::string::npos) {
        p_string.replace(pos, p_pattern.size(), p_replacement);
    }
}

char* StringUtils::Strdup(const char* p_source) {
#if USING(PLATFORM_WINDOWS)
    return _strdup(p_source);
#elif USING(PLATFORM_APPLE) || USING(PLATFORM_WASM)
    return strdup(p_source);
#else
#error Platform not supported
#endif
}

void StringUtils::Strcpy(char* p_dst, size_t p_dst_len, const char* p_src, size_t p_src_len) {
    if (!p_dst || p_dst_len == 0) return;

    if (!p_src || p_src_len == 0) {
        p_dst[0] = '\0';
        return;
    }

    const size_t copy_len = (p_dst_len - 1 < p_src_len) ? (p_dst_len - 1) : p_src_len;
    std::memcpy(p_dst, p_src, copy_len);
    p_dst[copy_len] = '\0';
    return;
}

std::string_view StringUtils::RemoveExtension(std::string_view p_file) {
    size_t dot_pos = p_file.find_last_of('.');
    return dot_pos == std::string_view::npos ? p_file : p_file.substr(0, dot_pos);
}

std::vector<std::string_view> StringUtils::Tokenize(std::string_view p_str) {
    std::vector<std::string_view> out;

    size_t i = 0;
    while (i < p_str.size()) {
        // skip spaces
        while (i < p_str.size() && std::isspace((unsigned char)p_str[i]))
            ++i;

        if (i >= p_str.size())
            break;

        size_t start = i;

        // read token
        while (i < p_str.size() && !std::isspace((unsigned char)p_str[i]))
            ++i;

        out.emplace_back(p_str.data() + start, i - start);
    }

    return out;
}

}  // namespace cave
