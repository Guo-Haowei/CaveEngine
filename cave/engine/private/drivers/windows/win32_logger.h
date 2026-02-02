#pragma once
#include "engine/private/core/logging/Logger.h"

namespace cave {

class Win32Logger : public ILogger {
public:
    void Print(LogLevel p_level, std::string_view p_message) override;

private:
    std::mutex m_consoleMutex;
};

}  // namespace cave
