// =============================================================================
// File: cave/core/diagnostics/Log.h
// =============================================================================
#pragma once
#include <format>
#include <string>

#include "cave/core/CoreExport.h"
#include "cave/core/typedefs.h"
#include "cave/core/diagnostics/LogChannel.h"

#define USE_LOG IN_USE

#if USING(USE_LOG)
#define LOG_TRACE(...) ::cave::LogImpl(::cave::LOG_LEVEL_TRACE, __VA_ARGS__)
#define LOG_INFO(...)  ::cave::LogImpl(::cave::LOG_LEVEL_INFO, __VA_ARGS__)
#define LOG_OK(...)    ::cave::LogImpl(::cave::LOG_LEVEL_OK, __VA_ARGS__)
#define LOG_WARN(...)  ::cave::LogImpl(::cave::LOG_LEVEL_WARN, __VA_ARGS__)
#define LOG_ERROR(...) ::cave::LogImpl(::cave::LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOG_FATAL(...) ::cave::LogImpl(::cave::LOG_LEVEL_FATAL, __VA_ARGS__)
#else
#define LOG_TRACE(...) (void)0
#define LOG(...)       (void)0
#define LOG_OK(...)    (void)0
#define LOG_WARN(...)  (void)0
#define LOG_ERROR(...) (void)0
#define LOG_FATAL(...) (void)0
#endif

namespace cave {

enum LogLevel : uint16_t {
    LOG_LEVEL_TRACE = 0x0001,
    LOG_LEVEL_INFO = 0x0002,
    LOG_LEVEL_OK = 0x0004,
    LOG_LEVEL_WARN = 0x0008,
    LOG_LEVEL_ERROR = 0x0010,
    LOG_LEVEL_FATAL = 0x0020,
    LOG_LEVEL_ALL = LOG_LEVEL_TRACE |
                    LOG_LEVEL_INFO |
                    LOG_LEVEL_OK |
                    LOG_LEVEL_WARN |
                    LOG_LEVEL_ERROR |
                    LOG_LEVEL_FATAL,
};
DEFINE_ENUM_BITWISE_OPERATIONS(LogLevel);

enum class LogChannel : uint16_t;

struct LogEvent {
    static inline constexpr int kMaxTimeString = 12;
    LogLevel level;
    LogChannel channel;
    char time_str[kMaxTimeString];  // HH:MM:SS.mm
    uint32_t repeat;
    int64_t timestamp_ms;
    std::string message;
};

CAVE_CORE_API void LogImpl(LogLevel level, LogChannel channel, std::string message);

static inline void LogImpl(LogLevel level, std::string message) {
    return LogImpl(level, LogChannel::Default, std::move(message));
}

template<typename... Args>
inline void LogImpl(LogLevel level, std::format_string<Args...> format, Args&&... args) {
    std::string message = std::format(format, std::forward<Args>(args)...);
    LogImpl(level, std::move(message));
}

template<typename... Args>
inline void LogImpl(LogLevel level, LogChannel channel, std::format_string<Args...> format, Args&&... args) {
    std::string message = std::format(format, std::forward<Args>(args)...);
    LogImpl(level, channel, std::move(message));
}

static inline const char* ToString(LogLevel level) {
    // clang-format off
    switch (level) {
        case LOG_LEVEL_TRACE:   return "TRACE";
        case LOG_LEVEL_INFO:    return "INFO ";
        case LOG_LEVEL_OK:      return "OK   ";
        case LOG_LEVEL_WARN:    return "WARN ";
        case LOG_LEVEL_ERROR:   return "ERROR";
        case LOG_LEVEL_FATAL:   return "FATAL";
        default:                return "?";
    }
    // clang-format on
}

static inline const char* ToString(LogChannel channel) {
    static constexpr const char* channels[] = {
#define CAVE_LOG_CHANNEL(ENUM, STR) STR,
        CAVE_LOG_CHANNEL_LIST
#undef CAVE_LOG_CHANNEL
    };
    static_assert(std::size(channels) == std::to_underlying(LogChannel::Count));
    return channels[std::to_underlying(channel)];
}

static inline std::string FormatLog(const LogEvent& log) {
    return std::format("{}  {}  {}  {}\n",
                       log.time_str,
                       ToString(log.level),
                       ToString(log.channel),
                       log.message);
}

}  // namespace cave
