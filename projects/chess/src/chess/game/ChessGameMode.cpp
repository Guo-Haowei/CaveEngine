#include "ChessGameMode.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"

#include "chess/game/ChessGameSession.h"

namespace chess {

using namespace cave;

ChessGameMode::ChessGameMode(SceneRuntime& runtime, IntentBus& intent_bus) {
    m_session = MakeOwner<ChessGameSession>(runtime, intent_bus);
}

ChessGameMode::~ChessGameMode() = default;

void ChessGameMode::onEnter() {
    m_session->onEnterBoot();
}

void ChessGameMode::onExit() {
    m_session.reset();
}

void ChessGameMode::tick(float) {
    m_session->tick();
}

}  // namespace chess
