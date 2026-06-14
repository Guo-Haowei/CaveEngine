#pragma once
#include <mutex>

#include "cave/core/diagnostics/ILogSink.h"
#include "cave/core/base/Singleton.h"

namespace cave {

class CompositeLogger : public ILogSink, public Singleton<CompositeLogger> {
public:
    void Submit(const LogEvent& log) override;

    void AddLogger(std::shared_ptr<ILogSink> logger);
    void AddChannel(LogLevel log) { m_channels |= log; }
    void RemoveChannel(LogLevel log) { m_channels &= ~log; }

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
        void Add(LogEvent log);
        void Clear() { logs.clear(); }
    };

    std::vector<std::shared_ptr<ILogSink>> m_loggers;

    GroupedLog m_all_logs;
    GroupedLog m_errors;
    GroupedLog m_warnings;

    Buffer m_buffer;

    std::atomic_uint32_t m_channels{ LOG_LEVEL_ALL };
};

}  // namespace cave
