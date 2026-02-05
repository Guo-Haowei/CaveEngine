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
    };

    DvarParser(std::span<std::string_view> p_commands, Source p_source = Source::None)
        : m_source(p_source), m_commands(p_commands) {}

    bool Parse();

    std::string_view GetError() const { return m_error; }

private:
    bool ProcessSetCmd();
    bool ProcessListCmd();

    bool OutOfBound();
    std::string_view Peek();
    std::string_view Consume();

    bool TryGetInt(int& p_out);
    bool TryGetFloat(float& p_out);
    bool TryGetString(std::string_view& p_out);

    const Source m_source;
    size_t m_cursor = 0;
    std::span<std::string_view>& m_commands;
    std::string m_error;
};

class DvarCache {
public:
    static void Serialize(std::string_view p_path);
    static void Deserialize(std::string_view p_path);
    static bool Parse(std::span<std::string_view> p_commands);
    static void DumpDvars();
};

}  // namespace cave
#endif
