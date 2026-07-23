#include "cave/core/diagnostics/CommandRegistry.h"

namespace cave {

void CommandRegistry::registerCmd(CommandDesc&& cmd) {
    for (const CommandDesc& desc : m_cmds) {
        if (desc.name == cmd.name) {
            LOG_ERROR("CommandRegistry::Register: command '{}' already registered", cmd.name);
            return;
        }
    }

    m_cmds.emplace_back(std::move(cmd));
}

const CommandDesc* CommandRegistry::findCmd(std::string_view name) const {
    for (const CommandDesc& cmd : m_cmds) {
        if (cmd.name == name) {
            return &cmd;
        }
    }
    return nullptr;
}

std::span<const CommandDesc> CommandRegistry::allCommands() const {
    return m_cmds;
}

void CommandRegistry::findByPrefix(std::string_view prefix, Vector<std::string_view>& out) const {
    for (const CommandDesc& cmd : m_cmds) {
        if (cmd.name.starts_with(prefix)) {
            out.push_back(cmd.name);
        }
    }
}

}  // namespace cave
