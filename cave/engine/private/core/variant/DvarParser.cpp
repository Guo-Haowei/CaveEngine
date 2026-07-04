#include "DvarParser.h"

#if USING(ENABLE_DVAR)

#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/core/diagnostics/ILogSink.h"

#include "engine/private/core/io/archive.h"

namespace cave {

#define TOKEN_EOF "<EOF>"

bool DvarParser::Parse() {
    for (;;) {
        std::string_view command = Peek();
        if (command == TOKEN_EOF) {
            return true;
        }

        if (command == "+set") {
            Consume();  // pop set
            if (!ParseSetCmd(m_error)) {
                return false;
            }
        } else {
            m_error = std::format("unknown command '{}'", command);
            return false;
        }
    }
}

bool DvarParser::ParseSetCmd(std::string& p_out) {
    std::string_view name = Consume();
    if (name == TOKEN_EOF) {
        p_out = "unexpected <EOF>";
        return false;
    }

    Dvar* dvar = Dvar::findDvar(std::string(name));
    if (dvar == nullptr) {
        p_out = std::format("dvar '{}' not found", name);
        return false;
    }

    VariantType type = dvar->type();
    bool ok = true;
    std::string_view str;
    int ix = 0, iy = 0, iz = 0, iw = 0;
    float fx = 0, fy = 0, fz = 0, fw = 0;
    size_t arg_start_index = m_cursor;
    const bool overriden = dvar->flags() & DVAR_FLAG_OVERRIDDEN;
    switch (type) {
        case VariantType::Int: {
            ok = ok && TryGetInt(ix);
            if (!overriden) {
                ok = ok && dvar->setInt(ix);
            }
        } break;
        case VariantType::Float: {
            ok = ok && TryGetFloat(fx);
            if (!overriden) {
                ok = ok && dvar->setFloat(fx);
            }
        } break;
        case VariantType::String: {
            ok = ok && TryGetString(str);
            if (!overriden) {
                ok = ok && dvar->setString(str);
            }
        } break;
        case VariantType::Vec2f: {
            ok = ok && TryGetFloat(fx);
            ok = ok && TryGetFloat(fy);
            if (!overriden) {
                ok = ok && dvar->setVec2f(fx, fy);
            }
        } break;
        case VariantType::Vec3f: {
            ok = ok && TryGetFloat(fx);
            ok = ok && TryGetFloat(fy);
            ok = ok && TryGetFloat(fz);
            if (!overriden) {
                ok = ok && dvar->setVec3f(fx, fy, fz);
            }
        } break;
        case VariantType::Vec4f: {
            ok = ok && TryGetFloat(fx);
            ok = ok && TryGetFloat(fy);
            ok = ok && TryGetFloat(fz);
            ok = ok && TryGetFloat(fw);
            if (!overriden) {
                ok = ok && dvar->setVec4f(fx, fy, fz, fw);
            }
        } break;
        case VariantType::Vec2i: {
            ok = ok && TryGetInt(ix);
            ok = ok && TryGetInt(iy);
            if (!overriden) {
                ok = ok && dvar->setVec2i(ix, iy);
            }
        } break;
        case VariantType::Vec3i: {
            ok = ok && TryGetInt(ix);
            ok = ok && TryGetInt(iy);
            ok = ok && TryGetInt(iz);
            if (!overriden) {
                ok = ok && dvar->setVec3i(ix, iy, iz);
            }
        } break;
        case VariantType::Vec4i: {
            ok = ok && TryGetInt(ix);
            ok = ok && TryGetInt(iy);
            ok = ok && TryGetInt(iz);
            ok = ok && TryGetInt(iw);
            if (!overriden) {
                ok = ok && dvar->setVec4i(ix, iy, iz, iw);
            }
        } break;
        default:
            CRASH_NOW();
            break;
    }

    if (!ok) {
        p_out = std::format("invalid arguments: {}", name);
        for (size_t i = arg_start_index; i < m_commands.size(); ++i) {
            p_out.push_back(' ');
            p_out.append(m_commands[i]);
        }
        return false;
    }

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

}  // namespace cave
#endif
