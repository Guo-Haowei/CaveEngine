#include "cave/core/diagnostics/CommandRegistry.h"

namespace cave {

void CommandRegistry::Register(CommandDesc p_cmd) {
    for (const CommandDesc& cmd : m_cmds) {
        if (cmd.name == p_cmd.name) {
            LOG_ERROR("CommandRegistry::Register: command '{}' already registered", p_cmd.name);
            return;
        }
    }

    m_cmds.emplace_back(std::move(p_cmd));
}

const CommandDesc* CommandRegistry::Find(std::string_view p_name) const {
    for (const CommandDesc& cmd : m_cmds) {
        if (cmd.name == p_name) {
            return &cmd;
        }
    }
    return nullptr;
}

std::span<const CommandDesc> CommandRegistry::Commands() const {
    return m_cmds;
}

void CommandRegistry::FindByPrefix(std::string_view p_prefix, std::vector<std::string_view>& p_out) const {
    for (const CommandDesc& cmd : m_cmds) {
        if (cmd.name.starts_with(p_prefix)) {
            p_out.push_back(cmd.name);
        }
    }
}

}  // namespace cave
