#include "cave/runtime/game/GameSession.h"

namespace cave {

void GameSession::setBool(StringId key, bool value) {
    m_variables[key] = Variant(value);
}

void GameSession::setInt(StringId key, int value) {
    m_variables[key] = Variant(value);
}

void GameSession::setFloat(StringId key, float value) {
    m_variables[key] = Variant(value);
}

void GameSession::setString(StringId key, std::string_view value) {
    m_variables[key] = Variant(value);
}

bool GameSession::getBool(StringId key, bool fallback) const {
    auto it = m_variables.find(key);
    return (it != m_variables.end()) ? it->second.asBool(fallback) : fallback;
}

int GameSession::getInt(StringId key, int fallback) const {
    auto it = m_variables.find(key);
    return (it != m_variables.end()) ? it->second.asInt(fallback) : fallback;
}

float GameSession::getFloat(StringId key, float fallback) const {
    auto it = m_variables.find(key);
    return (it != m_variables.end()) ? it->second.asFloat(fallback) : fallback;
}

std::string_view GameSession::getString(StringId key, std::string_view fallback) const {
    auto it = m_variables.find(key);
    return (it != m_variables.end()) ? it->second.asString(fallback) : fallback;
}

}  // namespace cave
