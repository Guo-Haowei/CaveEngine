// =============================================================================
// File: engine/private/runtime/gameplay/GameSession.cpp
// =============================================================================
#include "cave/runtime/gameplay/GameSession.h"

namespace cave {

GameSession::GameSession(GameModeFactory& p_factory)
    : m_factory(p_factory) {}

bool GameSession::Start(std::string_view p_mode_id) {
    Stop();

    auto mode = m_factory.Create(p_mode_id);
    if (!mode) {
        LOG_ERROR("GameSession::Start: mode '{}' not found", p_mode_id);
        return false;
    }

    m_active_id = std::string(p_mode_id);
    m_active = std::move(mode);

    m_active->OnEnter(*this);
    return true;
}

bool GameSession::RequestSwitch(std::string_view p_mode_id) {
    auto next = m_factory.Create(p_mode_id);
    if (!next) {
        return false;
    }

    m_pending_id = std::string(p_mode_id);
    m_pending = std::move(next);
    return true;
}

bool GameSession::CommitModeSwitch() {
    if (!m_pending) {
        return false;
    }

    if (m_active) {
        m_active->OnExit(*this);
    }

    m_active = std::move(m_pending);
    m_active_id = std::move(m_pending_id);
    m_pending_id.clear();

    m_active->OnEnter(*this);
    return true;
}

void GameSession::Stop() {
    m_pending.reset();
    m_pending_id.clear();

    if (m_active) {
        m_active->OnExit(*this);
        m_active.reset();
    }
    m_active_id.clear();
}

void GameSession::Tick(const GameFrameTime& p_frame) {
    if (m_active) {
        m_active->Tick(*this, p_frame);
    }
}

IPlayerAgent* GameSession::GetPlayer(uint32_t p_index) {
    if (DEV_VERIFY(p_index < m_players.size())) {
        return m_players[p_index].get();
    }

    return nullptr;
}

}  // namespace cave