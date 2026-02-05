// =============================================================================
// File: engine/public/cave/core/diagnostics/CommandRegistry.h
// =============================================================================
#pragma once
#include "Command.h"

namespace cave {

class CommandRegistry {
public:
    void Register(CommandDesc p_cmd);

    [[nodiscard]] const CommandDesc* Find(std::string_view p_name) const;

    [[nodiscard]] std::span<const CommandDesc> Commands() const;

    void FindByPrefix(std::string_view p_prefix, std::vector<std::string_view>& p_out) const;

private:
    std::vector<CommandDesc> m_cmds;
};

}  // namespace cave
