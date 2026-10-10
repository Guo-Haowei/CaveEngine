#pragma once
#include "cave/core/diagnostics/ILogSink.h"

namespace cave {

class Win32Logger : public ILogSink {
public:
    void submit(const LogEvent& log) override;

private:
    std::mutex m_console_mutex;
};

}  // namespace cave
