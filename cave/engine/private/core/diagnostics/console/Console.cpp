#include "Console.h"

#include "cave/core/diagnostics/CompositeLogger.h"
#include "cave/core/string/StringUtils.h"
#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/os/os.h"

namespace cave {

Console::Console(EngineServices& services) noexcept
    : m_services(services)
    , m_reg(services.commandRegistry()) {}

void Console::submitLine(std::string_view p_line) {
    if (p_line.empty()) return;
    std::vector<std::string_view> tokens = StringUtils::tokenize(p_line);
    if (tokens.empty()) return;

    std::span<const CommandDesc> cmds = m_reg.allCommands();
    bool ok = false;
    for (const CommandDesc& cmd : cmds) {
        if (cmd.name == tokens[0]) {
            CommandContext ctx{
                .log = LogWrapper(OS::singleton().logger()),
                .desc = cmd,
                .services = m_services,
            };

            ok = cmd.fn(ctx, { tokens });
            break;
        }
    }

    if (m_history.empty() || m_history.back() != p_line) {
        m_history.emplace_back(std::string(p_line));
    }
    resetNav();
    if (!ok) {
        LOG_ERROR(LogChannel::Console, "Failed to execute '{}'", p_line);
    }
}

Option<std::string_view> Console::prev() {
    if (m_history.empty()) {
        return None();
    }

    --m_index;
    if (m_index < 0) {
        m_index = 0;
    }

    std::string_view history = m_history[m_index];
    return Some(history);
}

Option<std::string_view> Console::next() {
    if (m_history.empty()) {
        return None();
    }

    ++m_index;
    if (m_index >= (int)m_history.size()) {
        --m_index;
    }

    std::string_view history = m_history[m_index];
    return Some(history);
}

}  // namespace cave
