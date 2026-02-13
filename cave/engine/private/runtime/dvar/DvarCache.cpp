#include "DvarCache.h"

#if USING(ENABLE_DVAR)

#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/core/diagnostics/ILogger.h"

#include "engine/private/core/io/archive.h"
#include "cave/core/string/StringUtils.h"
#include "engine/private/runtime/dvar/DvarParser.h"

namespace cave {

void DvarCache::Serialize(std::string_view p_path) {
    auto res = FileAccess::Open(p_path, FileAccess::WRITE);
    if (!res) {
        LOG_ERROR("{}", ToString(res.error()));
        return;
    }

    LOG("[dvar] serializing dvars");
    auto writer = std::move(*res);

    for (auto const& [key, dvar] : Dvar::s_map) {
        if (dvar->m_flags & DVAR_FLAG_CACHE) {
            auto line = std::format("+set {} {}\n", dvar->m_name, dvar->ValueToString());
            writer->WriteBuffer(line.data(), line.length());
        }
    }

    writer->Close();
}

void DvarCache::Deserialize(std::string_view p_path) {
    auto res = FileAccess::Open(p_path, FileAccess::READ);
    if (!res) {
        if (res.error()->value != ErrorCode::ERR_FILE_NOT_FOUND) {
            LOG_ERROR("{}", ToString(res.error()));
        }
        return;
    }

    auto reader = std::move(*res);
    const size_t size = reader->GetLength();
    std::string buffer;
    buffer.resize(size);
    reader->ReadBuffer(buffer.data(), size);
    reader->Close();

    std::vector<std::string_view> commands = StringUtils::Tokenize(buffer);
    DvarParser parser(commands, DvarParser::Source::Cache);
    if (!parser.Parse()) {
        LOG_ERROR("[dvar] Error: {}", parser.GetError());
    }
}

bool DvarCache::Parse(std::span<const std::string_view> p_commands) {
    DvarParser parser(p_commands, DvarParser::Source::CommandLine);
    bool ok = parser.Parse();
    if (!ok) {
        LOG_ERROR("[dvar] Error: {}", parser.GetError());
    }
    return ok;
}

void DvarCache::RegisterCmd(CommandRegistry& p_reg) {
    p_reg.Register({
        .name = "dvar.set",
        .help = "List registered dvars.",
        .usage = "Usage: dvar.set name [value]",
        .fn = [](CommandContext& p_ctx, const CommandArgs& p_args) {
            if (p_args.tokens.empty()) {
                p_ctx.logger.Print(LOG_LEVEL_ERROR, p_ctx.desc.usage);
                return false;
            }
            std::span<const std::string_view> args = p_args.tokens.subspan(1);
            DvarParser parser(args, DvarParser::Source::Console);

            std::string err;
            if (parser.ParseSetCmd(err)) return true;

            p_ctx.logger.Print(LOG_LEVEL_ERROR, err);
            return false;
        },
    });
    p_reg.Register({
        .name = "dvar.dump",
        .help = "Dump all registered dvars.",
        .usage = "dvar.dump",
        .fn = [](CommandContext& p_ctx, const CommandArgs&) {
            std::string msg;
            msg.reserve(512);
            msg.append("Dvar:");
            for (const auto& it : Dvar::s_map) {
                msg.append(std::format(
                    "\n -- {}, '{}', {}",
                    it.first,
                    it.second->ValueToString(),
                    it.second->GetDesc()));
            }
            msg.push_back('\n');

            p_ctx.logger.Print(LogLevel::LOG_LEVEL_NORMAL, msg);
            return true;
        },
    });
}

}  // namespace cave
#endif
