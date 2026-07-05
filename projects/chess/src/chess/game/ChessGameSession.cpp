#include "ChessGameSession.h"

#include "cave/runtime/framework/EngineServices.h"
#include "cave/runtime/controller/GridSelectController.h"
#include "cave/runtime/input/IGameInput.h"
#include "cave/runtime/intent/IntentBus.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneContext.h"
#include "cave/runtime/scene/SceneQuery.h"

#include "chess/agents/ChessAIAgent.h"
#include "chess/agents/LocalHumanAgent.h"
#include "chess/game/ChessGameClient.h"
#include "chess/game/ChessIntent.h"
#include "chess/game/ChessMatchAuthority.h"
#include "chess/presentation/ChessGridSelectorAdapter.h"
#include "chess/states/GameOverState.h"

namespace chess {

using namespace ::cave::literals;
using namespace ::cave;
using cave::math::Vec2i;
using core::Color;
using core::Square;

ChessGameSession::ChessGameSession() noexcept
    : m_phase(SessionPhase::AwaitPlayerInput) {}

ChessGameSession::~ChessGameSession() = default;

void ChessGameSession::tick(SceneContext& ctx) {
    switch (m_phase) {
#define SESSION_PHASE(Enum)  \
    case SessionPhase::Enum: \
        tick##Enum(ctx);     \
        break;
        SESSION_PHASE_LIST
#undef SESSION_PHASE
    }

    if (m_auth->gameOver()) {
        setPhase(SessionPhase::GameOver);
        return;
    }

    ctx.engine_services.intentBus().flush();

    // update client visual
    m_client->present();

    // @TODO: refactor this part
    if (m_selector) {
        Vec2i focused = m_selector->focus();
        Square square = Square::fromFileRank((uint8_t)focused.x, (uint8_t)focused.y);
        m_client->board_view().setHovered(square);
    }
}

void ChessGameSession::tickAwaitPlayerInput(SceneContext& ctx) {
    // @TODO: grid adapter should be owned by client/player?
    if (m_grid_adapter) {
        m_grid_adapter->tick(ctx);
    }

    // poll player intents
    auto side = m_auth->sideToMove();
    m_agents[std::to_underlying(side)]->tick(ctx);
}

void ChessGameSession::tickResolvingMove(SceneContext&) {
    setPhase(SessionPhase::Animating);
}

void ChessGameSession::tickAnimating(SceneContext& ctx) {
    if (isAnimating(ctx)) {
        return;
    }

    setPhase(SessionPhase::AwaitPlayerInput);
}

void ChessGameSession::tickGameOver(SceneContext& ctx) {
    if (isAnimating(ctx)) {
        return;
    }

    LOG_INFO(LogChannel::Game, "Game Over!");

    // auto state = std::make_unique<GameOverState>();
    // host_.intentBus().queue<ChessStateIntent>(std::move(state));
}

bool ChessGameSession::isAnimating(SceneContext& ctx) const {
    return ctx.query.componentCount(TransformAnimationComponent_Id) != 0;
}

auto ChessGameSession::createPlayer(Color side, PlayerKind kind)
    -> std::unique_ptr<IPlayerAgent> {
    switch (kind) {
        case PlayerKind::LocalHuman:
            return std::make_unique<LocalHumanAgent>(side);
        case PlayerKind::LocalAI:
            return std::make_unique<ChessAIAgent>(side, *m_client);
        case PlayerKind::RemoteNetwork:
            return nullptr;
        default:
            return nullptr;
    }
}

// @TODO: this should be configured by MainMenu?
void ChessGameSession::onEnterBoot(SceneContext& ctx) {
    MatchConfig config{};
    config.black = { PlayerKind::LocalAI };

    auto& intent_bus = ctx.engine_services.intentBus();
    m_auth = std::make_unique<ChessMatchAuthority>(intent_bus);
    m_client = std::make_unique<ChessGameClient>(intent_bus,
                                                 ctx.scene,
                                                 *this,
                                                 *m_auth);

    const PlayerKind white = config.white.kind;
    const PlayerKind black = config.black.kind;

    m_agents[0] = createPlayer(Color::White, white);
    m_agents[1] = createPlayer(Color::Black, black);

    const bool any_human = white == PlayerKind::LocalHuman || black == PlayerKind::LocalHuman;
    if (any_human) {
        m_grid_adapter = std::make_unique<ChessGridSelectorAdapter>(
            ctx,
            *m_client,
            m_client->board_view());

        cave::GridSelectController::Callbacks cbs = {
            .can_select = [this](int x, int y) { return m_grid_adapter->canSelect(x, y); },
            .on_select = [this](int x, int y) { m_grid_adapter->onSelect(x, y); },
            .can_drop = [this](int sx, int sy, int dx, int dy) { return m_grid_adapter->canDrop(sx, sy, dx, dy); },
            .on_drop = [this](int sx, int sy, int dx, int dy) { m_grid_adapter->onDrop(sx, sy, dx, dy); },
            .on_cancel = [this]() { m_grid_adapter->onCancel(); },
            .on_invalid = [this](int sx, int sy, int dx, int dy) { m_grid_adapter->onInvalid(sx, sy, dx, dy); }
        };

        m_selector = std::make_unique<cave::GridSelectController>(
            Vec2i(8, 8),
            std::move(cbs));

        m_grid_adapter->setController(m_selector.get());

        m_grid_adapter->setPlayerCb([this](Color side) -> LocalHumanAgent* {
            return dynamic_cast<LocalHumanAgent*>(m_agents[std::to_underlying(side)].get());
        });
    }

    m_client->onBoot();
}

#if USING(DEBUG_BUILD)
static const char* toString(SessionPhase phase) {
    switch (phase) {
#define SESSION_PHASE(Enum)  \
    case SessionPhase::Enum: \
        return #Enum;
        SESSION_PHASE_LIST
#undef SESSION_PHASE
        default:
            return "?";
    }
}
#endif

void ChessGameSession::setPhase(SessionPhase phase) {
    if (phase == m_phase) {
        return;
    }

#if USING(DEBUG_BUILD)
    auto msg = std::format("SessionPhase {} -> {}", toString(m_phase), toString(phase));
    LOG_TRACE(LogChannel::Game, std::move(msg));
#endif

    m_phase = phase;
}

}  // namespace chess
