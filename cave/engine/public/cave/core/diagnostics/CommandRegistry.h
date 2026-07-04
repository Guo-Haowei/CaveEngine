// =============================================================================
// File: cave/core/diagnostics/CommandRegistry.h
// =============================================================================
#pragma once
#include "cave/core/diagnostics/Command.h"

namespace cave {

class CommandRegistry {
public:
    void registerCmd(CommandDesc&& cmd);

    [[nodiscard]] const CommandDesc* findCmd(std::string_view name) const;

    [[nodiscard]] std::span<const CommandDesc> allCommands() const;

    void findByPrefix(std::string_view prefix, std::vector<std::string_view>& out) const;

private:
    std::vector<CommandDesc> cmds_;
};

}  // namespace cave
