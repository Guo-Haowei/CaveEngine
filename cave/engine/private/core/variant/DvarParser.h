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

    DvarParser(std::span<const std::string_view> commands, Source source = Source::None)
        : source_(source), commands_(commands) {}

    bool parse();
    bool parseSetCmd(std::string& out);

    std::string_view error() const { return error_; }

private:
    bool outOfBound();
    std::string_view peek();
    std::string_view consume();

    bool tryGetInt(int& out);
    bool tryGetFloat(float& out);
    bool tryGetString(std::string_view& out);

    const Source source_;
    size_t cursor_ = 0;
    std::span<const std::string_view>& commands_;
    std::string error_;
};

}  // namespace cave
#endif
