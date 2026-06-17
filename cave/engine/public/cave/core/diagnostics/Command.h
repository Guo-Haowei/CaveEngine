// =============================================================================
// File: cave/core/diagnostics/Command.h
// =============================================================================
#pragma once
#include <span>
#include "cave/core/typedefs.h"
#include "cave/core/diagnostics/LogWrapper.h"

#define USE_COMMAND IN_USE

namespace cave {

struct EngineServices;
struct CommandDesc;
class ILogSink;

struct CommandContext {
    LogWrapper log;
    const CommandDesc& desc;
    EngineServices& services;
};

struct CommandArgs {
    std::span<const std::string_view> tokens;
};

using CommandFn = bool (*)(CommandContext&, const CommandArgs&);
;

struct CommandDesc {
    std::string name;
    std::string help;
    std::string usage;
    CommandFn fn{};
};

}  // namespace cave
