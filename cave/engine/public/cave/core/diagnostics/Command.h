#pragma once

namespace cave {

class IApplication;
class ILogger;

struct CommandContext {
    ILogger& logger;
    IApplication& app;
};

struct CommandArgs {
    std::span<const std::string_view> tokens;
};

using CommandFn = std::function<void(CommandContext&, const CommandArgs&)>;

struct CommandDesc {
    std::string name;
    std::string help;
    std::string usage;
    CommandFn fn{};
};

}  // namespace cave
