// =============================================================================
// File: engine/runtime/gameplay/GameSession.cpp
// =============================================================================
#include "GameSession.h"

namespace cave {

GameSession::GameSession(GameModeFactory& p_factory)
    : m_factory(p_factory) {}

bool GameSession::Start(std::string_view p_mode_id) {
    Stop();

    auto mode = m_factory.Create(p_mode_id);
    if (!mode) {
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

void GameSession::Tick(const GameFrameTime& time) {
    if (m_active) {
        m_active->Tick(*this, time);
    }
}

}  // namespace cave