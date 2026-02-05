#pragma once

namespace cave {

class ILogger;

class ConsoleSink {
public:
    explicit ConsoleSink(ILogger& p_logger) noexcept
        : m_logger(p_logger) {}

    void Log(std::string_view p_message);

    void Warn(std::string_view p_message);

    void Error(std::string_view p_message);

private:
    ILogger& m_logger;
};

}  // namespace cave
