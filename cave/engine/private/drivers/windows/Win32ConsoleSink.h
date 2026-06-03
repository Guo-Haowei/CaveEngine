#pragma once
#include "engine/private/core/diagnostics/log_sink/Logger.h"

namespace cave {

class Win32Logger : public ILogSink {
public:
    void Submit(const LogEvent& p_log) override;

private:
    std::mutex m_console_mutex;
};

}  // namespace cave
