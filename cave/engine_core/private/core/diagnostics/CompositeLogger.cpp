#include <mutex>
#include <vector>

#include "cave/core/diagnostics/CompositeLogger.h"
#include "cave/core/error/ErrorMacros.h"

namespace cave {

// @TODO: fix this part
// #if USING(ENABLE_ASSERT)
// #define ASSERT_OPERATION_THREAD() DEV_ASSERT(thread::IsMainThread())
// #else
// #endif
#define ASSERT_OPERATION_THREAD() ((void)0)

struct GroupedLog {
    std::vector<LogEvent> logs;
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
    void addLevel(LogLevel level) { level_filter_ |= level; }
    void removeLevel(LogLevel level) { level_filter_ &= ~level; }

    void flush();

    void clearLog();

    std::span<const LogEvent> allLogs() const;
    std::span<const LogEvent> warningLogs() const;
    std::span<const LogEvent> errorLogs() const;

private:
    struct Buffer {
        std::vector<LogEvent> buffer;
        std::mutex mutex;
    };

    std::vector<std::unique_ptr<ILogSink>> loggers_;

    GroupedLog all_logs_;
    GroupedLog errors_;
    GroupedLog warnings_;

    Buffer buffer_;

    uint32_t level_filter_{ LOG_LEVEL_ALL };
};

CompositeLogger::CompositeLogger()
    : impl_(new Impl()) {
}

CompositeLogger::~CompositeLogger() {
    if (impl_) {
        delete impl_;
        impl_ = nullptr;
    }
}

void CompositeLogger::submit(const LogEvent& log) {
    impl_->submit(log);
}

void CompositeLogger::addLogger(std::unique_ptr<ILogSink>&& logger) {
    impl_->addLogger(std::move(logger));
}

void CompositeLogger::addLevel(LogLevel level) {
    impl_->addLevel(level);
}

void CompositeLogger::removeLevel(LogLevel level) {
    impl_->removeLevel(level);
}

void CompositeLogger::flush() {
    impl_->flush();
}

void CompositeLogger::clearLog() {
    impl_->clearLog();
}

std::span<const LogEvent> CompositeLogger::allLogs() const {
    return impl_->allLogs();
}

std::span<const LogEvent> CompositeLogger::warningLogs() const {
    return impl_->warningLogs();
}

std::span<const LogEvent> CompositeLogger::errorLogs() const {
    return impl_->errorLogs();
}

void CompositeLogger::Impl::addLogger(std::unique_ptr<ILogSink>&& logger) {
    loggers_.emplace_back(std::move(logger));
}

void CompositeLogger::Impl::submit(const LogEvent& log) {
    // @TODO: set verbose
    if (!(level_filter_ & log.level)) {
        return;
    }

    for (auto& logger : loggers_) {
        logger->submit(log);
    }

    buffer_.mutex.lock();
    buffer_.buffer.emplace_back(log);
    buffer_.mutex.unlock();
}

void CompositeLogger::Impl::flush() {
    ASSERT_OPERATION_THREAD();

    buffer_.mutex.lock();

    for (LogEvent& log : buffer_.buffer) {
        switch (log.level) {
            case LogLevel::LOG_LEVEL_FATAL:
            case LogLevel::LOG_LEVEL_ERROR: {
                errors_.add(log);
            } break;
            case LogLevel::LOG_LEVEL_WARN: {
                warnings_.add(log);
            } break;
            default:
                break;
        }
        all_logs_.add(std::move(log));
    }

    buffer_.buffer.clear();
    buffer_.mutex.unlock();
}

void CompositeLogger::Impl::clearLog() {
    ASSERT_OPERATION_THREAD();
    all_logs_.clear();
    errors_.clear();
    warnings_.clear();
}

std::span<const LogEvent> CompositeLogger::Impl::allLogs() const {
    ASSERT_OPERATION_THREAD();
    return all_logs_.logs;
}

std::span<const LogEvent> CompositeLogger::Impl::warningLogs() const {
    ASSERT_OPERATION_THREAD();
    return warnings_.logs;
}

std::span<const LogEvent> CompositeLogger::Impl::errorLogs() const {
    ASSERT_OPERATION_THREAD();
    return errors_.logs;
}

}  // namespace cave
