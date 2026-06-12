// =============================================================================
// File: cave/core/error/Result.h
// =============================================================================
#pragma once
#include <expected>

#include "Error.h"

namespace cave {

template<typename T>
using Result = std::expected<T, Error>;

template<typename T>
[[nodiscard]] constexpr inline auto CreateErrorArg(std::string_view file,
                                                   std::string_view function,
                                                   int line,
                                                   T value) {
    return std::unexpected(InternalError<T>(file, function, line, value));
};

template<typename T>
[[nodiscard]] constexpr inline auto CreateErrorArg(std::string_view file,
                                                   std::string_view function,
                                                   int line,
                                                   InternalError<T>& error) {
    return std::unexpected(InternalError<T>(file, function, line, std::move(error)));
};

template<typename T, typename... Args>
[[nodiscard]] constexpr inline auto CreateErrorArgs(std::string_view file,
                                                    std::string_view function,
                                                    int line,
                                                    T value,
                                                    std::format_string<Args...> format,
                                                    Args&&... args) {
    return std::unexpected(InternalError<T>(file, function, line, value, format, std::forward<Args>(args)...));
};

#define CAVE_ERROR_1(_1)                             ::cave::CreateErrorArg<ErrorCode>(__FILE__, __FUNCTION__, __LINE__, _1)
#define CAVE_ERROR_2(_1, _2)                         ::cave::CreateErrorArgs<ErrorCode>(__FILE__, __FUNCTION__, __LINE__, _1, _2)
#define CAVE_ERROR_3(_1, _2, _3)                     ::cave::CreateErrorArgs<ErrorCode>(__FILE__, __FUNCTION__, __LINE__, _1, _2, _3)
#define CAVE_ERROR_4(_1, _2, _3, _4)                 ::cave::CreateErrorArgs<ErrorCode>(__FILE__, __FUNCTION__, __LINE__, _1, _2, _3, _4)
#define CAVE_ERROR_5(_1, _2, _3, _4, _5)             ::cave::CreateErrorArgs<ErrorCode>(__FILE__, __FUNCTION__, __LINE__, _1, _2, _3, _4, _5)
#define CAVE_ERROR_6(_1, _2, _3, _4, _5, _6)         ::cave::CreateErrorArgs<ErrorCode>(__FILE__, __FUNCTION__, __LINE__, _1, _2, _3, _4, _5, _6)
#define CAVE_ERROR_7(_1, _2, _3, _4, _5, _6, _7)     ::cave::CreateErrorArgs<ErrorCode>(__FILE__, __FUNCTION__, __LINE__, _1, _2, _3, _4, _5, _6, _7)
#define CAVE_ERROR_8(_1, _2, _3, _4, _5, _6, _7, _8) ::cave::CreateErrorArgs<ErrorCode>(__FILE__, __FUNCTION__, __LINE__, _1, _2, _3, _4, _5, _6, _7, _8)
#define CAVE_ERROR(...)                              CAVE_MACRO_EXPAND(CAVE_GET_MACRO_8(__VA_ARGS__, CAVE_ERROR_8, CAVE_ERROR_7, CAVE_ERROR_6, CAVE_ERROR_5, CAVE_ERROR_4, CAVE_ERROR_3, CAVE_ERROR_2, CAVE_ERROR_1)(__VA_ARGS__))

}  // namespace cave
