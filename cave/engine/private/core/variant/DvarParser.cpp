#include "DvarParser.h"

#if USING(ENABLE_DVAR)

#include "cave/core/diagnostics/CommandRegistry.h"
#include "cave/core/diagnostics/ILogSink.h"

#include "engine/private/core/io/archive.h"

namespace cave {

#define TOKEN_EOF "<EOF>"

bool DvarParser::parse() {
    for (;;) {
        std::string_view command = peek();
        if (command == TOKEN_EOF) {
            return true;
        }

        if (command == "+set") {
            consume();  // pop set
            if (!parseSetCmd(error_)) {
                return false;
            }
        } else {
            error_ = std::format("unknown command '{}'", command);
            return false;
        }
    }
}

bool DvarParser::parseSetCmd(std::string& out) {
    std::string_view name = consume();
    if (name == TOKEN_EOF) {
        out = "unexpected <EOF>";
        return false;
    }

    Dvar* dvar = FindStaticDvar(name);
    if (dvar == nullptr) {
        out = std::format("dvar '{}' not found", name);
        return false;
    }

    VariantType type = dvar->type();
    bool ok = true;
    std::string_view str;
    int ix = 0, iy = 0, iz = 0, iw = 0;
    float fx = 0, fy = 0, fz = 0, fw = 0;
    size_t arg_start_index = cursor_;
    const bool overriden = dvar->flags() & DVAR_FLAG_OVERRIDDEN;
    switch (type) {
        case VariantType::Int: {
            ok = ok && tryGetInt(ix);
            if (!overriden) {
                ok = ok && dvar->setValue(ix);
            }
        } break;
        case VariantType::Float: {
            ok = ok && tryGetFloat(fx);
            if (!overriden) {
                ok = ok && dvar->setValue(fx);
            }
        } break;
        case VariantType::String: {
            ok = ok && tryGetString(str);
            if (!overriden) {
                ok = ok && dvar->setValue(str);
            }
        } break;
        case VariantType::Vec2f: {
            ok = ok && tryGetFloat(fx);
            ok = ok && tryGetFloat(fy);
            if (!overriden) {
                ok = ok && dvar->setValue(fx, fy);
            }
        } break;
        case VariantType::Vec3f: {
            ok = ok && tryGetFloat(fx);
            ok = ok && tryGetFloat(fy);
            ok = ok && tryGetFloat(fz);
            if (!overriden) {
                ok = ok && dvar->setValue(fx, fy, fz);
            }
        } break;
        case VariantType::Vec4f: {
            ok = ok && tryGetFloat(fx);
            ok = ok && tryGetFloat(fy);
            ok = ok && tryGetFloat(fz);
            ok = ok && tryGetFloat(fw);
            if (!overriden) {
                ok = ok && dvar->setValue(fx, fy, fz, fw);
            }
        } break;
        case VariantType::Vec2i: {
            ok = ok && tryGetInt(ix);
            ok = ok && tryGetInt(iy);
            if (!overriden) {
                ok = ok && dvar->setValue(ix, iy);
            }
        } break;
        case VariantType::Vec3i: {
            ok = ok && tryGetInt(ix);
            ok = ok && tryGetInt(iy);
            ok = ok && tryGetInt(iz);
            if (!overriden) {
                ok = ok && dvar->setValue(ix, iy, iz);
            }
        } break;
        case VariantType::Vec4i: {
            ok = ok && tryGetInt(ix);
            ok = ok && tryGetInt(iy);
            ok = ok && tryGetInt(iz);
            ok = ok && tryGetInt(iw);
            if (!overriden) {
                ok = ok && dvar->setValue(ix, iy, iz, iw);
            }
        } break;
        default:
            CRASH_NOW();
            break;
    }

    if (!ok) {
        out = std::format("invalid arguments: {}", name);
        for (size_t i = arg_start_index; i < commands_.size(); ++i) {
            out.push_back(' ');
            out.append(commands_[i]);
        }
        return false;
    }

    return true;
}

std::string_view DvarParser::peek() {
    if (outOfBound()) {
        return TOKEN_EOF;
    }

    return commands_[cursor_];
}

std::string_view DvarParser::consume() {
    if (outOfBound()) {
        return TOKEN_EOF;
    }

    return commands_[cursor_++];
}

bool DvarParser::tryGetInt(int& out) {
    if (outOfBound()) {
        return false;
    }
    std::string_view value = consume();
    if (value == "true") {
        out = 1;
    } else if (value == "false") {
        out = 0;
    } else {
        out = atoi(value.data());
    }
    return true;
}

bool DvarParser::tryGetFloat(float& out) {
    if (outOfBound()) {
        return false;
    }
    out = (float)atof(consume().data());
    return true;
}

bool DvarParser::tryGetString(std::string_view& out) {
    if (outOfBound()) {
        return false;
    }

    std::string_view next = consume();
    if (next.length() >= 2 && next.front() == '"' && next.back() == '"') {
        out = std::string_view(next.data() + 1, next.length() - 2);
    } else {
        out = std::string_view(next.data(), next.length());
    }

    return true;
}

bool DvarParser::outOfBound() {
    return cursor_ >= commands_.size();
}

}  // namespace cave
#endif
