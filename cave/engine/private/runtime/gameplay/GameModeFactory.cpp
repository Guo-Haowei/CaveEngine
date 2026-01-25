// =============================================================================
// File: engine/private/runtime/gameplay/GameModeFactory.cpp
// =============================================================================
#include "cave/runtime/gameplay/GameModeFactory.h"
#include "cave/runtime/gameplay/IGameMode.h"

namespace cave {

bool GameModeFactory::Register(std::string_view p_id, CreateFn p_create_fn, DestroyFn p_destroy_fn) {
    if (p_id.empty() || !p_create_fn || !p_destroy_fn) {
        return false;
    }
    std::string key(p_id);
    auto it = m_creators.find(key);
    if (it != m_creators.end()) {
        return false;
    }

    m_creators.emplace(std::move(key), CreatorEntry{ std::move(p_create_fn), std::move(p_destroy_fn) });
    return true;
}

GameModeFactory::GameModeRef GameModeFactory::Create(std::string_view p_id) const {
    auto it = m_creators.find(std::string(p_id));
    if (it == m_creators.end()) {
        return GameModeFactory::GameModeRef(nullptr, nullptr);
    }
    const CreatorEntry& e = it->second;
    return GameModeFactory::GameModeRef(e.create(), e.destroy);
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
