#include "DvarCache.h"

#if USING(ENABLE_DVAR)

#include "engine/private/core/io/archive.h"
#include "engine/private/core/string/StringUtils.h"

namespace cave {

#define TOKEN_EOF "<EOF>"

void DvarCache::Serialize(std::string_view p_path) {
    auto res = FileAccess::Open(p_path, FileAccess::WRITE);
    if (!res) {
        LOG_ERROR("{}", ToString(res.error()));
        return;
    }

    LOG("[dvar] serializing dvars");
    auto writer = std::move(*res);

    for (auto const& [key, dvar] : DynamicVariable::s_map) {
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

    // @TODO: use string_view instead
    std::vector<std::string_view> commands = StringUtils::Tokenize(buffer);

    DvarParser parser(commands, DvarParser::Source::Cache);
    if (!parser.Parse()) {
        LOG_ERROR("[dvar] Error: {}", parser.GetError());
    }
}

void DvarCache::DumpDvars() {
    for (const auto& it : DynamicVariable::s_map) {
        PRINT("-- {}, '{}'", it.first, it.second->GetDesc());
    }
}

//--------------------------------------------------------------------------------------------------
// Dynamic Varialbe Parser
//--------------------------------------------------------------------------------------------------

bool DvarParser::Parse() {
    for (;;) {
        std::string_view command = Peek();
        if (command == TOKEN_EOF) {
            return true;
        }

        if (command == "+set") {
            Consume();  // pop set
            if (!ProcessSetCmd()) {
                return false;
            }
        } else if (command == "+list") {
            Consume();
            ProcessListCmd();
            return true;
        } else {
            m_error = std::format("unknown command '{}'", command);
            return false;
        }
    }
}

bool DvarParser::ProcessSetCmd() {
    std::string_view name = Consume();
    if (name == TOKEN_EOF) {
        m_error = "unexpected <EOF>";
        return false;
    }

    DynamicVariable* dvar = DynamicVariable::FindDvar(std::string(name));
    if (dvar == nullptr) {
        m_error = std::format("dvar '{}' not found", name);
        return false;
    }

    VariantType type = dvar->GetType();
    bool ok = true;
    std::string_view str;
    int ix = 0, iy = 0, iz = 0, iw = 0;
    float fx = 0, fy = 0, fz = 0, fw = 0;
    size_t arg_start_index = m_cursor;
    const bool overriden = dvar->GetFlags() & DVAR_FLAG_OVERRIDEN;
    switch (type) {
        case VARIANT_TYPE_INT: {
            ok = ok && TryGetInt(ix);
            if (!overriden) {
                ok = ok && dvar->SetInt(ix);
            }
        } break;
        case VARIANT_TYPE_FLOAT: {
            ok = ok && TryGetFloat(fx);
            if (!overriden) {
                ok = ok && dvar->SetFloat(fx);
            }
        } break;
        case VARIANT_TYPE_STRING: {
            ok = ok && TryGetString(str);
            if (!overriden) {
                ok = ok && dvar->SetString(str);
            }
        } break;
        case VARIANT_TYPE_VEC2: {
            ok = ok && TryGetFloat(fx);
            ok = ok && TryGetFloat(fy);
            if (!overriden) {
                ok = ok && dvar->SetVector2f(fx, fy);
            }
        } break;
        case VARIANT_TYPE_VEC3: {
            ok = ok && TryGetFloat(fx);
            ok = ok && TryGetFloat(fy);
            ok = ok && TryGetFloat(fz);
            if (!overriden) {
                ok = ok && dvar->SetVector3f(fx, fy, fz);
            }
        } break;
        case VARIANT_TYPE_VEC4: {
            ok = ok && TryGetFloat(fx);
            ok = ok && TryGetFloat(fy);
            ok = ok && TryGetFloat(fz);
            ok = ok && TryGetFloat(fw);
            if (!overriden) {
                ok = ok && dvar->SetVector4f(fx, fy, fz, fw);
            }
        } break;
        case VARIANT_TYPE_IVEC2: {
            ok = ok && TryGetInt(ix);
            ok = ok && TryGetInt(iy);
            if (!overriden) {
                ok = ok && dvar->SetVector2i(ix, iy);
            }
        } break;
        case VARIANT_TYPE_IVEC3: {
            ok = ok && TryGetInt(ix);
            ok = ok && TryGetInt(iy);
            ok = ok && TryGetInt(iz);
            if (!overriden) {
                ok = ok && dvar->SetVector3i(ix, iy, iz);
            }
        } break;
        case VARIANT_TYPE_IVEC4: {
            ok = ok && TryGetInt(ix);
            ok = ok && TryGetInt(iy);
            ok = ok && TryGetInt(iz);
            ok = ok && TryGetInt(iw);
            if (!overriden) {
                ok = ok && dvar->SetVector4i(ix, iy, iz, iw);
            }
        } break;
        default:
            CRASH_NOW();
            break;
    }

    if (!ok) {
        m_error = std::format("invalid arguments: +set {}", name);
        for (size_t i = arg_start_index; i < m_commands.size(); ++i) {
            m_error.push_back(' ');
            m_error.append(m_commands[i]);
        }
        return false;
    }

    // @TODO: refactor
    switch (m_source) {
        case Source::Cache:
            dvar->PrintValueChange("cache");
            break;
        case Source::CommandLine:
            dvar->PrintValueChange("command line");
            break;
        default:
            break;
    }
    return true;
}

bool DvarParser::ProcessListCmd() {
    DvarCache::DumpDvars();
    return true;
}

std::string_view DvarParser::Peek() {
    if (OutOfBound()) {
        return TOKEN_EOF;
    }

    return m_commands[m_cursor];
}

std::string_view DvarParser::Consume() {
    if (OutOfBound()) {
        return TOKEN_EOF;
    }

    return m_commands[m_cursor++];
}

bool DvarParser::TryGetInt(int& p_out) {
    if (OutOfBound()) {
        return false;
    }
    std::string_view value = Consume();
    if (value == "true") {
        p_out = 1;
    } else if (value == "false") {
        p_out = 0;
    } else {
        p_out = atoi(value.data());
    }
    return true;
}

bool DvarParser::TryGetFloat(float& p_out) {
    if (OutOfBound()) {
        return false;
    }
    p_out = (float)atof(Consume().data());
    return true;
}

bool DvarParser::TryGetString(std::string_view& p_out) {
    if (OutOfBound()) {
        return false;
    }

    std::string_view next = Consume();
    if (next.length() >= 2 && next.front() == '"' && next.back() == '"') {
        p_out = std::string_view(next.data() + 1, next.length() - 2);
    } else {
        p_out = std::string_view(next.data(), next.length());
    }

    return true;
}

bool DvarParser::OutOfBound() {
    return m_cursor >= m_commands.size();
}

// @TODO: avoid using std::vector<std::string>, use string_view instead
bool DvarCache::Parse(std::span<std::string_view> p_commands) {
    DvarParser parser(p_commands, DvarParser::Source::CommandLine);
    bool ok = parser.Parse();
    if (!ok) {
        LOG_ERROR("[dvar] Error: {}", parser.GetError());
    }
    return ok;
}

}  // namespace cave
#endif
