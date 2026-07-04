#include "DvarCache.h"

#if USING(ENABLE_DVAR)

#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/core/diagnostics/ILogSink.h"

#include "engine/private/core/io/archive.h"
#include "cave/core/string/StringUtils.h"
#include "engine/private/runtime/dvar/DvarParser.h"

namespace cave {

void DvarCache::serialize(std::string_view path) {
    auto res = FileAccess::Open(path, FileAccess::WRITE);
    if (!res) {
        return;
    }

    LOG_INFO(LogChannel::Dvar, "Serializing");
    auto writer = std::move(*res);

    for (auto const& [key, dvar] : Dvar::s_map) {
        if (dvar->m_flags & DVAR_FLAG_CACHE) {
            auto line = std::format("+set {} {}\n", dvar->m_name, dvar->ValueToString());
            writer->WriteBuffer(line.data(), line.length());
        }
    }

    writer->Close();
}

void DvarCache::deserialize(std::string_view path) {
    auto res = FileAccess::Open(path, FileAccess::READ);
    if (!res) {
        if (res.error().value() != ErrorCode::ERR_FILE_NOT_FOUND) {
            LOG_ERROR(LogChannel::Dvar, "{}", ToString(res.error()));
        }
        return;
    }

    LOG_INFO(LogChannel::Dvar, "Deserializing");

    auto reader = std::move(*res);
    const size_t size = reader->GetLength();
    std::string buffer;
    buffer.resize(size);
    reader->ReadBuffer(buffer.data(), size);
    reader->Close();

    std::vector<std::string_view> commands = StringUtils::Tokenize(buffer);
    DvarParser parser(commands, DvarParser::Source::Cache);
    if (!parser.Parse()) {
        LOG_ERROR(LogChannel::Dvar, "Error: {}", parser.GetError());
    }
}

bool DvarCache::parse(std::span<const std::string_view> commands) {
    DvarParser parser(commands, DvarParser::Source::CommandLine);
    bool ok = parser.Parse();
    if (!ok) {
        LOG_ERROR(LogChannel::Dvar, "Error: {}", parser.GetError());
    }
    return ok;
}

}  // namespace cave
#endif
