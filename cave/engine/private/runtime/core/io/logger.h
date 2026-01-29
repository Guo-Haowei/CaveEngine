#pragma once
#include "cave/core/Print.h"
#include "cave/core/Singleton.h"

namespace cave {
// WORD is flags of FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
// clang-format off
//                  Level,              TAG         Ansi            DWORD
#define LOG_LEVEL_COLOR_LIST                                             \
    LOG_LEVEL_COLOR(LOG_LEVEL_VERBOSE,  "",         "\033[90m",     0x8) \
    LOG_LEVEL_COLOR(LOG_LEVEL_NORMAL,   "",         "\033[0m",      0x7) \
    LOG_LEVEL_COLOR(LOG_LEVEL_OK,       "[OK]",     "\033[92m",     0xA) \
    LOG_LEVEL_COLOR(LOG_LEVEL_WARN,     "[WARN] ",  "\033[93m",     0xE) \
    LOG_LEVEL_COLOR(LOG_LEVEL_ERROR,    "[ERROR]",  "\033[91m",     0xC) \
    LOG_LEVEL_COLOR(LOG_LEVEL_FATAL,    "[FATAL]",  "\033[101;30m", 0xC)
// clang-format on

struct LogEvent {
    LogLevel level;
    uint64_t id;
    std::string message;
};

class ILogger {
public:
    virtual ~ILogger() = default;

    virtual void Print(LogLevel p_level, std::string_view p_message) = 0;
};

class StdLogger : public ILogger {
public:
    virtual void Print(LogLevel p_level, std::string_view p_message) override;
};

class CompositeLogger : public ILogger, public Singleton<CompositeLogger> {
public:
    void Print(LogLevel p_level, std::string_view p_message) override;

    void AddLogger(std::shared_ptr<ILogger> p_logger);
    void AddChannel(LogLevel p_log) { m_channels |= p_log; }
    void RemoveChannel(LogLevel p_log) { m_channels &= ~p_log; }

    void Flush();

    void ClearLog();

    const std::vector<LogEvent>& GetAllLogs() const;
    const std::vector<LogEvent>& GetWarningLogs() const;
    const std::vector<LogEvent>& GetErrorLogs() const;

private:
    struct Buffer {
        std::vector<LogEvent> buffer;
        std::mutex mutex;
    };

    std::vector<std::shared_ptr<ILogger>> m_loggers;

    std::vector<LogEvent> m_all_logs;
    std::vector<LogEvent> m_errors;
    std::vector<LogEvent> m_warnings;

    Buffer m_buffer;

    std::atomic_uint32_t m_channels{ LOG_LEVEL_ALL };
    std::atomic_uint64_t m_log_id{ 0 };
};

}  // namespace cave
