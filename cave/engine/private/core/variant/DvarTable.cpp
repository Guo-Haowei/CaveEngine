#include "DvarTable.h"

#include "cave/core/string/StringUtils.h"

#include "engine/private/core/io/file_access.h"
#include "engine/private/core/variant/DvarParser.h"

namespace cave {

static DvarTable s_global;

DvarTable& DvarTable::global() {
    return s_global;
}

Dvar* DvarTable::find(std::string_view sv) {
    auto it = m_entry_lookup.find(sv);
    if (it == m_entry_lookup.end()) {
        return nullptr;
    }

    if (DEV_VERIFY(it->second < m_storage.size())) {
        return m_storage[it->second].get();
    }

    return nullptr;
}

bool DvarTable::registerStatic(Dvar* dvar) {
    DEV_ASSERT(dvar);

    return registerImpl(dvar->name(),
                        {
                            .external = dvar,
                            .owned = nullptr,
                        });
}

bool DvarTable::registerDynamic(Owner<Dvar>&& dvar) {
    DEV_ASSERT(dvar);

    String name = dvar->name();
    return registerImpl(name,
                        {
                            .external = nullptr,
                            .owned = std::move(dvar),
                        });
}

bool DvarTable::registerImpl(const String& name, Entry&& entry) {
    const uint32_t slot = static_cast<uint32_t>(m_storage.size());
    auto [it, ok] = m_entry_lookup.try_emplace(name, slot);
    if (!ok) {
        LOG_ERROR(LogChannel::Dvar, "dvar {} already registered!", name);
        return false;
    }

    m_storage.push_back(std::move(entry));
    return true;
}

void DvarTable::serialize(std::string_view path) {
    auto res = FileAccess::Open(path, FileAccess::WRITE);
    if (!res) {
        return;
    }

    LOG_INFO(LogChannel::Dvar, "Serializing");
    auto writer = std::move(*res);

    // @TODO: unify serialization (YAML)
    for (const Entry& entry : m_storage) {
        const Dvar* dvar = entry.get();
        if (DEV_VERIFY(dvar)) {
            if (dvar->flags() & DVAR_FLAG_CACHE) {
                auto line = std::format("+set {} {}\n",
                                        dvar->name(),
                                        dvar->variant().toString());
                writer->WriteBuffer(line.data(), line.length());
            }
        }
    }

    writer->Close();
}

void DvarTable::deserialize(std::string_view path) {
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

    std::vector<std::string_view> commands = StringUtils::tokenize(buffer);
    DvarParser parser(commands, DvarParser::Source::Cache);
    if (!parser.parse()) {
        LOG_ERROR(LogChannel::Dvar, "Error: {}", parser.error());
    }
}

bool DvarTable::parse(std::span<std::string_view> commands) {
    DvarParser parser(commands, DvarParser::Source::CommandLine);
    bool ok = parser.parse();
    if (!ok) {
        LOG_ERROR(LogChannel::Dvar, "Error: {}", parser.error());
    }
    return ok;
}

}  // namespace cave
