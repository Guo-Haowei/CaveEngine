#pragma once
#include "engine/private/core/diagnostics/logger/Logger.h"

namespace cave {

class Win32Logger : public ILogger {
public:
    void Print(const LogEvent& p_log) override;

private:
    std::mutex m_consoleMutex;
};

}  // namespace cave
