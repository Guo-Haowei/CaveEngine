#include "logger.h"

#include "engine/private/core/os/threads.h"

namespace cave {

#if USING(ENABLE_ASSERT)
#define ASSERT_OPERATION_THREAD() DEV_ASSERT(thread::IsMainThread())
#else
#define ASSERT_OPERATION_THREAD() ((void)0)
#endif

void StdLogger::Print(LogLevel p_level, std::string_view p_message) {
    const char* tag = "";
    switch (p_level) {
#define LOG_LEVEL_COLOR(LEVEL, TAG, ANSI, WINCOLOR) \
    case LEVEL:                                     \
        tag = TAG;                                  \
        break;
        LOG_LEVEL_COLOR_LIST
#undef LOG_LEVEL_COLOR
        default:
            break;
    }

    // @TODO: stderr vs stdout
    FILE* file = stdout;
    fflush(file);

    fprintf(file, "%s%.*s", tag, static_cast<int>(p_message.length()), p_message.data());
    fflush(file);
}

void CompositeLogger::AddLogger(std::shared_ptr<ILogger> p_logger) {
    m_loggers.emplace_back(p_logger);
}

void CompositeLogger::Print(LogLevel p_level, std::string_view p_message) {
    // @TODO: set verbose
    if (!(m_channels & p_level)) {
        return;
    }

    for (auto& logger : m_loggers) {
        logger->Print(p_level, p_message);
    }

    LogEvent log = {
        .level = p_level,
        .id = m_log_id.fetch_add(1),
        .message = std::string(p_message),
    };

    m_buffer.mutex.lock();
    m_buffer.buffer.emplace_back(std::move(log));
    m_buffer.mutex.unlock();
}

void CompositeLogger::Flush() {
    ASSERT_OPERATION_THREAD();

    m_buffer.mutex.lock();

    for (LogEvent& log : m_buffer.buffer) {
        switch (log.level) {
            case LogLevel::LOG_LEVEL_FATAL:
            case LogLevel::LOG_LEVEL_ERROR: {
                m_errors.emplace_back(log);
            } break;
            case LogLevel::LOG_LEVEL_WARN: {
                m_warnings.emplace_back(log);
            } break;
            default:
                break;
        }
        m_all_logs.emplace_back(std::move(log));
    }

    m_buffer.buffer.clear();
    m_buffer.mutex.unlock();
}

void CompositeLogger::ClearLog() {
    ASSERT_OPERATION_THREAD();
    m_all_logs.clear();
}

const std::vector<LogEvent>& CompositeLogger::GetAllLogs() const {
    ASSERT_OPERATION_THREAD();
    return m_all_logs;
}

const std::vector<LogEvent>& CompositeLogger::GetWarningLogs() const {
    ASSERT_OPERATION_THREAD();
    return m_warnings;
}

const std::vector<LogEvent>& CompositeLogger::GetErrorLogs() const {
    ASSERT_OPERATION_THREAD();
    return m_errors;
}

}  // namespace cave
