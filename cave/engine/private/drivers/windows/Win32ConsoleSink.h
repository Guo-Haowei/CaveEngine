#pragma once
#include "cave/core/diagnostics/ILogSink.h"

namespace cave {

class Win32Logger : public ILogSink {
public:
    void Submit(const LogEvent& log) override;

private:
    std::mutex console_mutex_;
};

}  // namespace cave
