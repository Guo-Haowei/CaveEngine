#include "ChessGameMode.h"

#include <format>

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/diagnostics/ILogSink.h"
#include "cave/core/ErrorMacros.h"
#include "cave/core/typedefs.h"
#include "cave/game/IHostServices.h"

#include "chess/game/ChessGameSession.h"
#include "chess/game/ChessIntent.h"
#include "chess/states/MainMenuState.h"

namespace chess {

using namespace cave;

ChessGameMode::ChessGameMode(IHostServices& p_host)
    : m_host(p_host)
    , m_intent(p_host.intentDispatcher())
    , m_debug_id(MakeDebugId(this)) {
    m_intent.addHandler<ChessStateIntent>(this);
}

ChessGameMode::~ChessGameMode() {
    m_intent.removeHandler<ChessStateIntent>(this);
}

void ChessGameMode::OnEnter(IHostServices& p_host) {
    m_state = std::make_unique<MainMenuState>();
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

bool ChessGameMode::handleIntent(cave::Intent& p_intent) {
    if (auto intent = dynamic_cast<ChessStateIntent*>(&p_intent)) {
        CommitStateChange(std::move(intent->state_));
        return true;
    }

    return false;
}

void ChessGameMode::CommitStateChange(std::unique_ptr<IChessGameState>&& p_new_state) {
    DEV_ASSERT(p_new_state != nullptr);

    if (m_state) {
        m_state->OnExit(m_host);
    }

    m_state = std::move(p_new_state);
    m_state->OnEnter(m_host);
}

}  // namespace chess
