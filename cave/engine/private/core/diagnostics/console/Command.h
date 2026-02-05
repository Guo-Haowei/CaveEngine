#pragma once

namespace cave {

class ConsoleSink;

struct CommandContext {
};

struct CommandArgs {
    std::span<const std::string_view> tokens;
};

using CommandFn = void (*)(const CommandContext&, const CommandArgs&, ConsoleSink&);

struct CommandDesc {
    std::string name;
    std::string help;
    std::string usage;
    CommandFn fn{};
};

}  // namespace cave
