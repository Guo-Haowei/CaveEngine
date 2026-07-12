#include "GameplayState.h"

#include "chess/game/ChessGameSession.h"

namespace chess {

using namespace ::cave;

GameplayState::GameplayState(SceneRuntime& runtime,
                             IntentBus& intent_bus) noexcept
    : IChessGameState(runtime, intent_bus) {}

GameplayState::~GameplayState() = default;

void GameplayState::onEnter() {
    m_session = MakeOwner<ChessGameSession>(m_runtime, m_intent_bus);
    m_session->onEnterBoot();
}

void GameplayState::onExit() {
    m_session.reset();
}

void GameplayState::tick(float) {
    m_session->tick();
}

}  // namespace chess
