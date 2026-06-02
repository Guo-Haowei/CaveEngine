#pragma once
#include "cave/core/diagnostics/ILogger.h"
#include "cave/core/Singleton.h"

namespace cave {
// @TODO: rename the level

// WORD is flags of FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
// clang-format off
//                  Level,              TAG       Ansi            DWORD
#define LOG_LEVEL_COLOR_LIST                                           \
    LOG_LEVEL_COLOR(LOG_LEVEL_VERBOSE,  "TRACE",  "\033[90m",     0x8) \
    LOG_LEVEL_COLOR(LOG_LEVEL_NORMAL,   "INFO ",  "\033[0m",      0x7) \
    LOG_LEVEL_COLOR(LOG_LEVEL_OK,       "OK   ",  "\033[92m",     0xA) \
    LOG_LEVEL_COLOR(LOG_LEVEL_WARN,     "WARN ",  "\033[93m",     0xE) \
    LOG_LEVEL_COLOR(LOG_LEVEL_ERROR,    "ERROR",  "\033[91m",     0xC) \
    LOG_LEVEL_COLOR(LOG_LEVEL_FATAL,    "FATAL",  "\033[101;30m", 0xC)
// clang-format on

// @TODO: fix this
using LogEvent = Log;

class StdLogger : public ILogger {
public:
    virtual void Print(const Log& p_log) override;
};

class CompositeLogger : public ILogger, public Singleton<CompositeLogger> {
public:
    void Print(const Log& p_log) override;

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

    struct GroupedLog {
        std::vector<LogEvent> logs;
        void Add(LogEvent p_log);
        void Clear() { logs.clear(); }
    };

    std::vector<std::shared_ptr<ILogger>> m_loggers;

    GroupedLog m_all_logs;
    GroupedLog m_errors;
    GroupedLog m_warnings;

    Buffer m_buffer;

    std::atomic_uint32_t m_channels{ LOG_LEVEL_ALL };
    std::atomic_uint64_t m_log_id{ 0 };
};

const char* ToString(LogLevel p_level);

}  // namespace cave
