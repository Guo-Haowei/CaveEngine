#include "ChessGameMode.h"

#include <format>

#include "cave/core/diagnostics/ILogger.h"
#include "cave/core/ErrorMacros.h"
#include "cave/core/typedefs.h"
#include "cave/game/IHostServices.h"

#include "ChessGameSession.h"

namespace chess {

using namespace cave;

class IChessGameState {
public:
    virtual ~IChessGameState() = default;

    virtual void OnEnter(cave::IHostServices& p_host) {}
    virtual void OnExit(cave::IHostServices& p_host) {}

    virtual void Tick(cave::IHostServices& p_host, const cave::FrameTime& p_time) = 0;

    virtual const char* DebugName() = 0;
};

// =============================================================================
// MainMenuState
// =============================================================================
class MainMenuState final : public IChessGameState {
public:
    void Tick(cave::IHostServices& p_host, const cave::FrameTime& p_time) final;

    const char* DebugName() final {
        return "MainMenu";
    }
};

void MainMenuState::Tick(cave::IHostServices& p_host, const cave::FrameTime& p_time) {
    unused(p_host);
    unused(p_time);

    // @TODO: show menu buttons
    // @TODO: transit to gameplay once play button is clicked
    // @TODO: configure player (AI vs human)
}

// =============================================================================
// GameplayState
// =============================================================================
class GameplayState final : public IChessGameState {
public:
    void Tick(cave::IHostServices& p_host, const cave::FrameTime& p_time) final;

    const char* DebugName() final {
        return "GamePlay";
    }
};

void GameplayState::Tick(cave::IHostServices& p_host, const cave::FrameTime& p_time) {
    // @TODO: move session here
    unused(p_host);
    unused(p_time);
}

// =============================================================================
// ChessGameMode
// =============================================================================
ChessGameMode::ChessGameMode() = default;

ChessGameMode::~ChessGameMode() = default;

void ChessGameMode::OnEnter(IHostServices& p_host) {
    // @TODO: move session to PlayState
    m_session = std::make_unique<ChessGameSession>();

    m_pending_state = std::make_unique<MainMenuState>();
}

void ChessGameMode::OnExit(IHostServices& p_host) {
    unused(p_host);

    m_session.reset();
}

void ChessGameMode::Tick(IHostServices& p_host, const FrameTime& p_time) {
    m_session->Tick(p_host);

    if (m_pending_state) {
        CommitStateChange(p_host);
    }

    m_current_state->Tick(p_host, p_time);
}

void ChessGameMode::CommitStateChange(cave::IHostServices& p_host) {
    DEV_ASSERT(m_pending_state != nullptr);
    const char* current_name = m_current_state ? m_current_state->DebugName() : "(null)";
    const char* pending_name = m_pending_state->DebugName();
    p_host.Log().Print(LogLevel::LOG_LEVEL_NORMAL, std::format("ChessGameMode::CommitStateChange: {} -> {}", current_name, pending_name));

    if (m_current_state) {
        m_current_state->OnExit(p_host);
    }

    m_current_state = std::move(m_pending_state);
    m_current_state->OnEnter(p_host);
}

}  // namespace chess
