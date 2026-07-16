// =============================================================================
// File: cave/runtime/game/GameSession.h
// =============================================================================
#pragma once
#include "cave/core/string/StringId.h"
#include "cave/core/variant/Variant.h"

namespace cave {

class GameSession {
public:
    void setBool(StringId key, bool value);
    void setInt(StringId key, int value);
    void setFloat(StringId key, float value);
    void setString(StringId key, std::string_view value);

    bool getBool(StringId key, bool fallback = false) const;
    int getInt(StringId key, int llback = 0) const;
    float getFloat(StringId key, float fallback = 0.0f) const;
    std::string_view getString(StringId key, std::string_view fallback = "") const;

private:
    HashMap<StringId, Variant> m_variables;
};

}  // namespace cave
