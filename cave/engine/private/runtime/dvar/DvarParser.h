#pragma once
#include "Dvar.h"

#if USING(ENABLE_DVAR)
namespace cave {

class DvarParser {
public:
    enum class Source : uint8_t {
        None,
        CommandLine,
        Cache,
        Console,
    };

    DvarParser(std::span<const std::string_view> p_commands, Source p_source = Source::None)
        : m_source(p_source), m_commands(p_commands) {}

    bool Parse();
    bool ParseSetCmd(std::string& p_out);

    std::string_view GetError() const { return m_error; }

private:
    bool OutOfBound();
    std::string_view Peek();
    std::string_view Consume();

    bool TryGetInt(int& p_out);
    bool TryGetFloat(float& p_out);
    bool TryGetString(std::string_view& p_out);

    const Source m_source;
    size_t m_cursor = 0;
    std::span<const std::string_view>& m_commands;
    std::string m_error;
};

}  // namespace cave
#endif
