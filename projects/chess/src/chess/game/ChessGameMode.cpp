#include "ChessGameMode.h"

#include <format>

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/diagnostics/ILogSink.h"
#include "cave/core/error/ErrorMacros.h"
#include "cave/core/typedefs.h"
#include "cave/runtime/framework/EngineServices.h"

#include "chess/game/ChessGameSession.h"
#include "chess/game/ChessIntent.h"
#include "chess/states/MainMenuState.h"

namespace chess {

using namespace cave;

ChessGameMode::ChessGameMode(IntentBus& intent_bus)
    : m_intent_bus(intent_bus)
    , m_debug_id(MakeDebugId(this)) {
    m_intent_bus.addHandler<ChessStateIntent>(this);
}

ChessGameMode::~ChessGameMode() {
    m_intent_bus.removeHandler<ChessStateIntent>(this);
}

void ChessGameMode::onEnter() {
    m_state = std::make_unique<MainMenuState>(m_intent_bus);
    m_state->onEnter();
}

void ChessGameMode::onExit() {
}

void ChessGameMode::tick(float dt) {
    if (DEV_VERIFY(m_state)) {
        m_state->tick(dt);
        m_intent_bus.flush();
    }
}

bool ChessGameMode::handleIntent(Intent& intent) {
    if (auto state_intent = dynamic_cast<ChessStateIntent*>(&intent)) {
        commitStateChange(std::move(state_intent->m_state));
        return true;
    }

    return false;
}

void ChessGameMode::commitStateChange(Owner<IChessGameState>&& new_state) {
    DEV_ASSERT(new_state != nullptr);

    if (m_state) {
        m_state->onExit();
    }

    m_state = std::move(new_state);
    m_state->onEnter();
}

}  // namespace chess
