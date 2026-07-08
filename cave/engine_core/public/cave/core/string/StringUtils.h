// =============================================================================
// File: cave/core/string/StringUtils.h
// =============================================================================
#pragma once
#include <cstdarg>
#include <string>
#include <string_view>
#include <vector>

#include "cave/core/typedefs.h"
#include "cave/core/containers/Containers.h"
#include "cave/core/CoreExport.h"

namespace cave {

#if USING(PLATFORM_WINDOWS)
#define DELIMITER_CHAR '\\'
#define DELIMITER_STR  "\\"
#else
#define DELIMITER_CHAR '/'
#define DELIMITER_STR  "/"
#endif

class CAVE_CORE_API StringSplitter {
public:
    explicit StringSplitter(const char* str) {
        fast_ = str;
        slow_ = nullptr;
    }

    std::string_view advance(char c) {
        slow_ = fast_;
        fast_ = strchr(fast_, c);
        if (fast_ != nullptr) {
            return std::string_view(slow_, fast_++);
        }
        return slow_;
    }

    bool canAdvance() const {
        return fast_ != nullptr && *fast_ != 0;
    }

private:
    const char* fast_;
    const char* slow_;
};

class CAVE_CORE_API StringUtils {
public:
    static bool isNullOrEmpty(const char* str) {
        return !str || *str == '\0';
    }

    static bool equal(const char* str1, const char* str2);

    static void replaceFirst(String& string,
                             std::string_view pattern,
                             std::string_view replace);

    static char* strdup(const char* source);

    template<int N>
    static int sprintf(char (&buffer)[N], const char* format, ...) {
        va_list args;
        va_start(args, format);
        int result = vsnprintf(buffer, N, format, args);
        va_end(args);
        return result;
    }

    static void strcpy(char* dst, size_t dst_len, const char* src, size_t src_len);

    template<size_t N>
    static void strcpy(char (&dst)[N], const char* src) {
        StringUtils::strcpy(dst, N, src, strlen(src));
    }

    template<size_t N>
    static void strcpy(char (&dst)[N], const char* src, size_t src_len) {
        StringUtils::strcpy(dst, N, src, src_len);
    }

    template<size_t N>
    static void strcpy(char (&dst)[N], std::string_view src) {
        StringUtils::strcpy(dst, N, src.data(), src.size());
    }

    static constexpr bool isDigit(const char c) {
        return c >= '0' && c <= '9';
    }

    static constexpr bool isHex(char c) {
        return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }

    static constexpr char hexToInt(char c) {
        if (isDigit(c)) {
            return c - '0';
        }
        if (c >= 'a' && c <= 'f') {
            return c - 'a' + 10;
        }
        if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        }
        return -1;
    }

    static std::string_view removeExtension(std::string_view file);

    static constexpr std::string_view findLastOf(std::string_view sv, char pattern) {
        const size_t found = sv.find_last_of(pattern);
        return found == std::string_view::npos ? "" : std::string_view(sv.data() + found, sv.size() - found);
    }

    static constexpr std::string_view basePath(std::string_view sv, char delimiter = DELIMITER_CHAR) {
        const size_t found = sv.find_last_of(delimiter);
        return found == std::string_view::npos ? "" : std::string_view(sv.data(), found);
    }

    static constexpr std::string_view fileName(std::string_view sv, char delimiter = DELIMITER_CHAR) {
        auto result = findLastOf(sv, delimiter);
        return result == "" ? sv : std::string_view(result.data() + 1, result.size() - 1);
    }

    static constexpr std::string_view extension(std::string_view sv) {
        return findLastOf(sv, '.');
    }

    static std::vector<std::string_view> tokenize(std::string_view sv);
};

}  // namespace cave
