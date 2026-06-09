#include "GameplayState.h"

#include "chess/game/ChessGameSession.h"

namespace chess {

GameplayState::GameplayState() noexcept = default;

GameplayState::~GameplayState() = default;

void GameplayState::OnEnter(cave::IHostServices& p_host) {
    m_session = std::make_unique<ChessGameSession>(p_host);
    m_session->onEnterBoot();
}

void GameplayState::OnExit(cave::IHostServices& p_host) {
    unused(p_host);

    m_session.reset();
}

void GameplayState::Tick(cave::IHostServices&, const cave::FrameTime&) {

    m_session->tick();
}

}  // namespace chess
