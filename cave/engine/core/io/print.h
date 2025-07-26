#pragma once
#include "engine/math/math.h"

#define LOG_VERBOSE(...)   ::cave::LogImpl(::cave::LOG_LEVEL_VERBOSE, __VA_ARGS__)
#define LOG(...)           ::cave::LogImpl(::cave::LOG_LEVEL_NORMAL, __VA_ARGS__)
#define LOG_OK(...)        ::cave::LogImpl(::cave::LOG_LEVEL_OK, __VA_ARGS__)
#define LOG_WARN(...)      ::cave::LogImpl(::cave::LOG_LEVEL_WARN, __VA_ARGS__)
#define LOG_ERROR(...)     ::cave::LogImpl(::cave::LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_FATAL(...)     ::cave::LogImpl(::cave::LOG_LEVEL_FATAL, __VA_ARGS__)
#define PRINT_VERBOSE(...) ::cave::PrintImpl(::cave::LOG_LEVEL_VERBOSE, __VA_ARGS__)
#define PRINT(...)         ::cave::PrintImpl(::cave::LOG_LEVEL_NORMAL, __VA_ARGS__)
#define PRINT_OK(...)      ::cave::PrintImpl(::cave::LOG_LEVEL_OK, __VA_ARGS__)
#define PRINT_WARN(...)    ::cave::PrintImpl(::cave::LOG_LEVEL_WARN, __VA_ARGS__)
#define PRINT_ERROR(...)   ::cave::PrintImpl(::cave::LOG_LEVEL_ERROR, __VA_ARGS__)
#define PRINT_FATAL(...)   ::cave::PrintImpl(::cave::LOG_LEVEL_FATAL, __VA_ARGS__)

namespace cave {

enum LogLevel : uint8_t {
    LOG_LEVEL_VERBOSE = BIT(1),
    LOG_LEVEL_NORMAL = BIT(2),
    LOG_LEVEL_OK = BIT(3),
    LOG_LEVEL_WARN = BIT(4),
    LOG_LEVEL_ERROR = BIT(5),
    LOG_LEVEL_FATAL = BIT(6),
    LOG_LEVEL_ALL = LOG_LEVEL_VERBOSE |
                    LOG_LEVEL_NORMAL |
                    LOG_LEVEL_OK |
                    LOG_LEVEL_WARN |
                    LOG_LEVEL_ERROR |
                    LOG_LEVEL_FATAL,
};
DEFINE_ENUM_BITWISE_OPERATIONS(LogLevel);

void PrintImpl(LogLevel p_level, const std::string& p_message);

template<typename... Args>
inline void PrintImpl(LogLevel p_level, std::format_string<Args...> p_format, Args&&... p_args) {
    std::string message = std::format(p_format, std::forward<Args>(p_args)...);
    PrintImpl(p_level, message);
}

void LogImpl(LogLevel p_level, const std::string& p_message);

template<typename... Args>
inline void LogImpl(LogLevel p_level, std::format_string<Args...> p_format, Args&&... p_args) {
    std::string message = std::format(p_format, std::forward<Args>(p_args)...);
    LogImpl(p_level, message);
}

}  // namespace cave
