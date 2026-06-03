// =============================================================================
// File: engine/public/cave/core/diagnostics/Command.h
// =============================================================================
#pragma once

namespace cave {

class IApplication;
class ILogSink;
struct CommandDesc;

struct CommandContext {
    ILogSink& sink;
    const CommandDesc& desc;
    IApplication& app;
};

struct CommandArgs {
    std::span<const std::string_view> tokens;
};

using CommandFn = std::function<bool(CommandContext&, const CommandArgs&)>;

struct CommandDesc {
    std::string name;
    std::string help;
    std::string usage;
    CommandFn fn{};
};

}  // namespace cave
