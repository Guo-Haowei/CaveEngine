// =============================================================================
// File: engine/private/runtime/gameplay/GameModeFactory.cpp
// =============================================================================
#include "GameModeFactory.h"

#include "IGameMode.h"

namespace cave {

bool GameModeFactory::Register(std::string_view p_id, CreatorFn p_fn) {
    if (p_id.empty() || !p_fn) {
        return false;
    }
    std::string key(p_id);
    auto it = m_creators.find(key);
    if (it != m_creators.end()) {
        return false;
    }
    m_creators.emplace(std::move(key), std::move(p_fn));
    return true;
}

std::unique_ptr<IGameMode> GameModeFactory::Create(std::string_view p_id) const {
    auto it = m_creators.find(std::string(p_id));
    if (it == m_creators.end()) {
        return nullptr;
    }
    return (it->second)();
}

void GameModeFactory::ListIds(std::vector<std::string>& p_out_ids) const {
    p_out_ids.clear();
    p_out_ids.reserve(m_creators.size());
    for (const auto& kv : m_creators) {
        p_out_ids.push_back(kv.first);
    }
    std::sort(p_out_ids.begin(), p_out_ids.end());
}

}  // namespace cave
