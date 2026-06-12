// =============================================================================
// File: cave/core/diagnostics/LogWrapper.h
// =============================================================================
#pragma once
#include "cave/core/diagnostics/ILogSink.h"

namespace cave {

class CAVE_CORE_API LogWrapper {
public:
    explicit LogWrapper(ILogSink& p_sink) noexcept
        : sink_(p_sink) {}

    void Trace(LogChannel p_channel, std::string&& p_message) {
        Log(LOG_LEVEL_TRACE, p_channel, std::move(p_message));
    }

    void Info(LogChannel p_channel, std::string&& p_message) {
        Log(LOG_LEVEL_INFO, p_channel, std::move(p_message));
    }

    void Ok(LogChannel p_channel, std::string&& p_message) {
        Log(LOG_LEVEL_OK, p_channel, std::move(p_message));
    }

    void Warn(LogChannel p_channel, std::string&& p_message) {
        Log(LOG_LEVEL_WARN, p_channel, std::move(p_message));
    }

    void Error(LogChannel p_channel, std::string&& p_message) {
        Log(LOG_LEVEL_ERROR, p_channel, std::move(p_message));
    }

private:
    void Log(LogLevel p_level, LogChannel p_channel, std::string&& p_message);

    ILogSink& sink_;
};

}  // namespace cave