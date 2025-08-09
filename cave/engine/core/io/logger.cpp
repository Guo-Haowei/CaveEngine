#include "logger.h"

#include "engine/core/base/ring_buffer.h"

namespace cave {

static_assert(sizeof(CompositeLogger::Log) == CompositeLogger::kLogStructSize);

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

    m_log_history_mutex.lock();
    m_log_history.push_back({});
    auto& log_history = m_log_history.back();
    log_history.level = p_level;
    log_history.id = m_log_id.fetch_add(1);
    strncpy(log_history.buffer, p_message.data(), sizeof(log_history.buffer) - 1);
    m_log_history_mutex.unlock();
}

void CompositeLogger::ClearLog() {
    m_log_history_mutex.lock();
    m_log_history.clear();
    m_log_history_mutex.unlock();
}

void CompositeLogger::RetrieveLog(std::vector<Log>& p_out_logs) {
    m_log_history_mutex.lock();

    p_out_logs.reserve(m_log_history.size());
    for (auto& log : m_log_history) {
        p_out_logs.push_back(log);
    }

    m_log_history_mutex.unlock();
}

}  // namespace cave
