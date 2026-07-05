#include "ChessGameMode.h"

#include <format>

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/diagnostics/ILogSink.h"
#include "cave/core/error/ErrorMacros.h"
#include "cave/core/typedefs.h"
#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/scene/SceneContext.h"

#include "chess/game/ChessGameSession.h"
#include "chess/game/ChessIntent.h"
#include "chess/states/GameplayState.h"

namespace chess {

using namespace cave;

ChessGameMode::ChessGameMode(SceneContext& ctx)
    : m_intent_bus(ctx.engine_services.intentBus())
    , m_debug_id(MakeDebugId(this)) {
    m_intent_bus.addHandler<ChessStateIntent>(this);
}

ChessGameMode::~ChessGameMode() {
    m_intent_bus.removeHandler<ChessStateIntent>(this);
}

void ChessGameMode::onEnter(SceneContext& ctx) {
    m_state = std::make_unique<GameplayState>();
    m_state->onEnter(ctx);
}

void ChessGameMode::onExit() {
}

void ChessGameMode::tick(SceneContext& ctx, float dt) {
    if (DEV_VERIFY(m_state)) {
        m_state->tick(ctx, dt);
    }
}

bool ChessGameMode::handleIntent(Intent& p_intent) {
    if (auto intent = dynamic_cast<ChessStateIntent*>(&p_intent)) {
        // commitStateChange(std::move(intent->state_));
        return true;
    }

    return false;
}

void ChessGameMode::commitStateChange(SceneContext& ctx,
                                      std::unique_ptr<IChessGameState>&& new_state) {
    DEV_ASSERT(new_state != nullptr);
    unused(ctx);
    unused(new_state);

    // if (m_state) {
    //     m_state->OnExit(ctx);
    // }

    // m_state = std::move(new_state);
    // m_state->OnEnter(ctx);
}

}  // namespace chess
