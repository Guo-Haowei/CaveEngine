#include "Console.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/diagnostics/logger/Logger.h"

namespace cave {

// @TODO: move to StringUtil
static std::vector<std::string_view> Tokenize(std::string_view p_line) {
    std::vector<std::string_view> out;

    size_t i = 0;
    while (i < p_line.size()) {
        // skip spaces
        while (i < p_line.size() && std::isspace((unsigned char)p_line[i]))
            ++i;

        if (i >= p_line.size())
            break;

        size_t start = i;

        // read token
        while (i < p_line.size() && !std::isspace((unsigned char)p_line[i]))
            ++i;

        out.emplace_back(p_line.data() + start, i - start);
    }

    return out;
}

Console::Console(IApplication& p_app) noexcept
    : m_app(p_app)
    , m_reg(p_app.CommandRegistry()) {}

void Console::SubmitLine(std::string_view p_line) {
    if (p_line.empty()) return;
    std::vector<std::string_view> tokens = Tokenize(p_line);
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
}

}  // namespace cave
