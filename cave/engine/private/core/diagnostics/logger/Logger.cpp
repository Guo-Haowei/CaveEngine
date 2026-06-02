#include "logger.h"

#include "engine/private/core/os/threads.h"

namespace cave {

#if USING(ENABLE_ASSERT)
#define ASSERT_OPERATION_THREAD() DEV_ASSERT(thread::IsMainThread())
#else
#define ASSERT_OPERATION_THREAD() ((void)0)
#endif

void ILogger::Print(LogLevel p_level, std::string p_message) {
    Log log = BuildLog(p_level, std::move(p_message));
    Print(log);
}

void StdLogger::Print(const Log& p_log) {
    const char* tag = ToString(p_log.level);

    // @TODO: stderr vs stdout
    FILE* file = stdout;
    fflush(file);

    fprintf(file, "%s%s", tag, p_log.message.c_str());
    fflush(file);
}

void CompositeLogger::GroupedLog::Add(LogEvent p_log) {
    if (!logs.empty()) {
        LogEvent& log = logs.back();
        if (log.message == p_log.message && log.level == p_log.level) {
            ++log.repeat;
            return;
        }
    }

    logs.push_back(std::move(p_log));
}

void CompositeLogger::AddLogger(std::shared_ptr<ILogger> p_logger) {
    m_loggers.emplace_back(p_logger);
}

void CompositeLogger::Print(const Log& p_log) {
    // @TODO: set verbose
    if (!(m_channels & p_log.level)) {
        return;
    }

    for (auto& logger : m_loggers) {
        logger->Print(p_log);
    }

    m_buffer.mutex.lock();
    m_buffer.buffer.emplace_back(p_log);
    m_buffer.mutex.unlock();
}

void CompositeLogger::Flush() {
    ASSERT_OPERATION_THREAD();

    m_buffer.mutex.lock();

    for (LogEvent& log : m_buffer.buffer) {
        switch (log.level) {
            case LogLevel::LOG_LEVEL_FATAL:
            case LogLevel::LOG_LEVEL_ERROR: {
                m_errors.Add(log);
            } break;
            case LogLevel::LOG_LEVEL_WARN: {
                m_warnings.Add(log);
            } break;
            default:
                break;
        }
        m_all_logs.Add(std::move(log));
    }

    m_buffer.buffer.clear();
    m_buffer.mutex.unlock();
}

void CompositeLogger::ClearLog() {
    ASSERT_OPERATION_THREAD();
    m_all_logs.Clear();
    m_errors.Clear();
    m_warnings.Clear();
}

const std::vector<LogEvent>& CompositeLogger::GetAllLogs() const {
    ASSERT_OPERATION_THREAD();
    return m_all_logs.logs;
}

const std::vector<LogEvent>& CompositeLogger::GetWarningLogs() const {
    ASSERT_OPERATION_THREAD();
    return m_warnings.logs;
}

const std::vector<LogEvent>& CompositeLogger::GetErrorLogs() const {
    ASSERT_OPERATION_THREAD();
    return m_errors.logs;
}

const char* ToString(LogLevel p_level) {
    switch (p_level) {
#define LOG_LEVEL_COLOR(LEVEL, TAG, ANSI, WINCOLOR) \
    case LEVEL:                                     \
        return TAG;
        LOG_LEVEL_COLOR_LIST
#undef LOG_LEVEL_COLOR
        default:
            return "";
    }
}

}  // namespace cave
