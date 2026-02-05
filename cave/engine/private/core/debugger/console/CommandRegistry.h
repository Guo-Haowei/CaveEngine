#pragma once
#include "Command.h"

namespace cave::debug {

class CommandRegistry {
public:
    void Register(CommandDesc p_cmd);

    const CommandDesc* Find(std::string_view p_name) const;

    std::span<const CommandDesc> Commands() const;

    void FindByPrefix(std::string_view p_prefix, std::vector<std::string_view>& p_out) const;

private:
    std::vector<CommandDesc> m_cmds;
};

}  // namespace cave::debug
