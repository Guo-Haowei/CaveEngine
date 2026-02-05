#include "Console.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/diagnostics/logger/Logger.h"
#include "engine/private/core/string/StringUtils.h"

namespace cave {

Console::Console(IApplication& p_app) noexcept
    : m_app(p_app)
    , m_reg(p_app.CommandRegistry()) {}

void Console::SubmitLine(std::string_view p_line) {
    if (p_line.empty()) return;
    std::vector<std::string_view> tokens = StringUtils::Tokenize(p_line);
    if (tokens.empty()) return;

    std::span<const CommandDesc> cmds = m_reg.Commands();
    for (const CommandDesc& cmd : cmds) {
        if (cmd.name == tokens[0]) {
            CommandContext ctx{
                .logger = CompositeLogger::GetSingleton(),
                .app = m_app,
            };

            cmd.fn(ctx, { tokens });
            break;
        }
    }

    if (m_history.empty() || m_history.back() != p_line) {
        m_history.emplace_back(std::string(p_line));
    }
    ResetNav();
}

Option<std::string_view> Console::Prev() {
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

Option<std::string_view> Console::Next() {
    if (m_history.empty()) {
        return None();
    }

    ++m_index;
    if (m_index >= (int)m_history.size() - 1) {
        --m_index;
    }

    std::string_view history = m_history[m_index];
    return Some(history);
}

}  // namespace cave
