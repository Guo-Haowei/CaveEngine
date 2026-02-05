#include "ConsoleSink.h"

#include "cave/core/diagnostics/ILogger.h"

namespace cave {

void ConsoleSink::Log(std::string_view p_message) {
    m_logger.Print(LogLevel::LOG_LEVEL_NORMAL, p_message);
}

void ConsoleSink::Warn(std::string_view p_message) {
    m_logger.Print(LogLevel::LOG_LEVEL_WARN, p_message);
}

void ConsoleSink::Error(std::string_view p_message) {
    m_logger.Print(LogLevel::LOG_LEVEL_ERROR, p_message);
}

}  // namespace cave
