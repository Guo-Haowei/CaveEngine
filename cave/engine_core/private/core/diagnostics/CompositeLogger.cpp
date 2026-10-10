#include "cave/core/containers/Containers.h"
#include "cave/core/diagnostics/CompositeLogger.h"
#include "cave/core/error/ErrorMacros.h"

#include <mutex>

namespace cave {

#define ASSERT_OPERATION_THREAD() ((void)0)

struct GroupedLog {
    Vector<LogEvent> logs;
    void add(LogEvent log);
    void clear() { logs.clear(); }
};

bool operator==(const LogEvent& lhs, const LogEvent& rhs) {
    return lhs.level == rhs.level &&
           lhs.channel == rhs.channel &&
           lhs.message == rhs.message;
}

void GroupedLog::add(LogEvent log) {
    if (!logs.empty()) {
        if (logs.back() == log) {
            ++logs.back().repeat;
            return;
        }
    }

    logs.push_back(std::move(log));
}

class CompositeLogger::Impl {
public:
    void submit(const LogEvent& log);

    void addLogger(std::unique_ptr<ILogSink>&& logger);
    void addLevel(LogLevel level) { m_level_filter |= level; }
    void removeLevel(LogLevel level) { m_level_filter &= ~level; }

    void flush();

    void clearLog();

    std::span<const LogEvent> allLogs() const;
    std::span<const LogEvent> warningLogs() const;
    std::span<const LogEvent> errorLogs() const;

private:
    struct Buffer {
        Vector<LogEvent> buffer;
        std::mutex mutex;
    };

    Vector<Owner<ILogSink>> m_loggers;

    GroupedLog m_all_logs;
    GroupedLog m_errors;
    GroupedLog m_warnings;

    Buffer m_buffer;

    uint32_t m_level_filter{ LOG_LEVEL_ALL };
};

CompositeLogger::CompositeLogger()
    : m_impl(new Impl) {
}

CompositeLogger::~CompositeLogger() {
    if (m_impl) {
        delete m_impl;
        m_impl = nullptr;
    }
}

void CompositeLogger::submit(const LogEvent& log) {
    m_impl->submit(log);
}

void CompositeLogger::addLogger(Owner<ILogSink>&& logger) {
    m_impl->addLogger(std::move(logger));
}

void CompositeLogger::addLevel(LogLevel level) {
    m_impl->addLevel(level);
}

void CompositeLogger::removeLevel(LogLevel level) {
    m_impl->removeLevel(level);
}

void CompositeLogger::flush() {
    m_impl->flush();
}

void CompositeLogger::clearLog() {
    m_impl->clearLog();
}

std::span<const LogEvent> CompositeLogger::allLogs() const {
    return m_impl->allLogs();
}

std::span<const LogEvent> CompositeLogger::warningLogs() const {
    return m_impl->warningLogs();
}

std::span<const LogEvent> CompositeLogger::errorLogs() const {
    return m_impl->errorLogs();
}

void CompositeLogger::Impl::addLogger(Owner<ILogSink>&& logger) {
    m_loggers.emplace_back(std::move(logger));
}

void CompositeLogger::Impl::submit(const LogEvent& log) {
    // @TODO: set verbose
    if (!(m_level_filter & log.level)) {
        return;
    }

    for (auto& logger : m_loggers) {
        logger->submit(log);
    }

    m_buffer.mutex.lock();
    m_buffer.buffer.emplace_back(log);
    m_buffer.mutex.unlock();
}

void CompositeLogger::Impl::flush() {
    ASSERT_OPERATION_THREAD();

    m_buffer.mutex.lock();

    for (LogEvent& log : m_buffer.buffer) {
        switch (log.level) {
            case LogLevel::LOG_LEVEL_FATAL:
            case LogLevel::LOG_LEVEL_ERROR: {
                m_errors.add(log);
            } break;
            case LogLevel::LOG_LEVEL_WARN: {
                m_warnings.add(log);
            } break;
            default:
                break;
        }
        m_all_logs.add(std::move(log));
    }

    m_buffer.buffer.clear();
    m_buffer.mutex.unlock();
}

void CompositeLogger::Impl::clearLog() {
    ASSERT_OPERATION_THREAD();
    m_all_logs.clear();
    m_errors.clear();
    m_warnings.clear();
}

std::span<const LogEvent> CompositeLogger::Impl::allLogs() const {
    ASSERT_OPERATION_THREAD();
    return m_all_logs.logs;
}

std::span<const LogEvent> CompositeLogger::Impl::warningLogs() const {
    ASSERT_OPERATION_THREAD();
    return m_warnings.logs;
}

std::span<const LogEvent> CompositeLogger::Impl::errorLogs() const {
    ASSERT_OPERATION_THREAD();
    return m_errors.logs;
}

}  // namespace cave
