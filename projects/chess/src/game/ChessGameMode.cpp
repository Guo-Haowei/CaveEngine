#include "ChessGameMode.h"

#include <format>

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/diagnostics/ILogSink.h"
#include "cave/core/ErrorMacros.h"
#include "cave/core/typedefs.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/framework/IInputService.h"
#include "cave/runtime/framework/IUIRuntime.h"

#include "ChessGameSession.h"
#include "ChessIntent.h"
#include "IChessGameState.h"

namespace chess {

using namespace cave;

class MainMenuState final : public IChessGameState {
public:
    using IChessGameState::IChessGameState;

    void OnEnter(cave::IHostServices& p_host) final;

    void Tick(cave::IHostServices& p_host, const cave::FrameTime& p_time) final;

    const char* DebugName() final {
        return "MainMenu";
    }
};

class GameplayState final : public IChessGameState {
public:
    using IChessGameState::IChessGameState;

    void OnEnter(cave::IHostServices& p_host) final;
    void OnExit(cave::IHostServices& p_host) final;

    void Tick(cave::IHostServices& p_host, const cave::FrameTime& p_time) final;

    const char* DebugName() final {
        return "GamePlay";
    }

private:
    std::unique_ptr<ChessGameSession> m_session;
};

// =============================================================================
// MainMenuState
// =============================================================================
void MainMenuState::OnEnter(cave::IHostServices& p_host) {
    p_host.Log().Info(LogChannel::Game, "Press 'Enter' to play.");
}

void MainMenuState::Tick(cave::IHostServices& p_host, const cave::FrameTime& p_time) {
    unused(p_time);

    cave::IUIRuntime& ui = p_host.UI();

    ui.BeginView(p_host.GetViewId());
    const float offset_x = 760.0f;
    const float offset_y = 200.0f;
    if (ui.Button(1, { offset_x, offset_y, 400, 100 })) {
        auto gameplay = std::make_unique<GameplayState>(m_game);
        p_host.Intent().Queue<ChessStateIntent>(std::move(gameplay));
    }
    if (ui.Button(2, { offset_x, offset_y + 200, 400, 100 })) {
        p_host.Log().Ok(LogChannel::Game, "UI Button 2 clicked");
    }
    if (ui.Button(3, { offset_x, offset_y + 400, 400, 100 })) {
        p_host.Log().Ok(LogChannel::Game, "UI Button 3 clicked");
    }
    ui.EndView();
}

// =============================================================================
// GameplayState
// =============================================================================
void GameplayState::OnEnter(cave::IHostServices& p_host) {
    m_session = std::make_unique<ChessGameSession>();
    m_session->OnEnterBoot(p_host);
}

void GameplayState::OnExit(cave::IHostServices& p_host) {
    unused(p_host);

    m_session.reset();
}

void GameplayState::Tick(cave::IHostServices& p_host, const cave::FrameTime& p_time) {
    unused(p_time);

    m_session->Tick(p_host);
}

// =============================================================================
// ChessGameMode
// =============================================================================
ChessGameMode::ChessGameMode(IHostServices& p_host)
    : m_host(p_host)
    , m_intent(p_host.Intent())
    , m_debug_id(MakeDebugId(this)) {
    m_intent.AddHandler<ChessStateIntent>(this);
}

ChessGameMode::~ChessGameMode() {
    m_intent.RemoveHandler<ChessStateIntent>(this);
}

void ChessGameMode::OnEnter(IHostServices& p_host) {
    m_state = std::make_unique<MainMenuState>(*this);
    m_state->OnEnter(p_host);
}

void ChessGameMode::OnExit(IHostServices& p_host) {
    unused(p_host);
}

void ChessGameMode::Tick(IHostServices& p_host, const FrameTime& p_time) {
    if (DEV_VERIFY(m_state)) {
        m_state->Tick(p_host, p_time);
    }
}

bool ChessGameMode::HandleIntent(cave::Intent& p_intent) {
    if (auto intent = dynamic_cast<ChessStateIntent*>(&p_intent)) {
        CommitStateChange(std::move(intent->state));
        return true;
    }

    return false;
}

void ChessGameMode::CommitStateChange(std::unique_ptr<IChessGameState>&& p_new_state) {
    DEV_ASSERT(p_new_state != nullptr);
    const char* current_name = m_state ? m_state->DebugName() : "(null)";
    const char* pending_name = p_new_state->DebugName();

    if (m_state) {
        m_state->OnExit(m_host);
    }

    m_state = std::move(p_new_state);
    m_state->OnEnter(m_host);
}

}  // namespace chess
