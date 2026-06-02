// =============================================================================
// File: engine/public/cave/core/diagnostics/Log.h
// =============================================================================
#pragma once
#include <format>
#include <string>

#include "cave/core/typedefs.h"
#include "cave/core/math/Utils.h"

#define USE_LOG IN_USE

#if USING(USE_LOG)
#define LOG_VERBOSE(...) ::cave::LogImpl(::cave::LOG_LEVEL_VERBOSE, __VA_ARGS__)
#define LOG(...)         ::cave::LogImpl(::cave::LOG_LEVEL_NORMAL, __VA_ARGS__)
#define LOG_OK(...)      ::cave::LogImpl(::cave::LOG_LEVEL_OK, __VA_ARGS__)
#define LOG_WARN(...)    ::cave::LogImpl(::cave::LOG_LEVEL_WARN, __VA_ARGS__)
#define LOG_ERROR(...)   ::cave::LogImpl(::cave::LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_FATAL(...)   ::cave::LogImpl(::cave::LOG_LEVEL_FATAL, __VA_ARGS__)
#else
#define LOG_VERBOSE(...) (void)0
#define LOG(...)         (void)0
#define LOG_OK(...)      (void)0
#define LOG_WARN(...)    (void)0
#define LOG_ERROR(...)   (void)0
#define LOG_FATAL(...)   (void)0
#endif

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

enum class LogChannel : uint8_t;

struct Log {
    static inline constexpr int kMaxTimeString = 10;
    LogLevel level;
    LogChannel channel;
    char time_str[kMaxTimeString];  // HH:MM:SS
    uint32_t repeat;
    int64_t timestamp_ms;
    std::string message;
};

Log BuildLog(LogLevel p_level, std::string p_message);

void LogImpl(LogLevel p_level, std::string p_message);

template<typename... Args>
inline void LogImpl(LogLevel p_level, std::format_string<Args...> p_format, Args&&... p_args) {
    std::string message = std::format(p_format, std::forward<Args>(p_args)...);
    LogImpl(p_level, std::move(message));
}

}  // namespace cave
