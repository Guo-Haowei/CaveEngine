#include "ChessGameSession.h"

#include "cave/game/IHostServices.h"
#include "cave/runtime/controller/GridSelectController.h"
#include "cave/runtime/input/IGameInput.h"
#include "cave/runtime/intent/IntentDispatcher.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
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

ChessGameSession::ChessGameSession(cave::IHostServices& host) noexcept
    : host_(host)
    , phase_(SessionPhase::AwaitPlayerInput) {}

ChessGameSession::~ChessGameSession() = default;

void ChessGameSession::tick() {
    switch (phase_) {
#define SESSION_PHASE(Enum)  \
    case SessionPhase::Enum: \
        tick##Enum();        \
        break;
        SESSION_PHASE_LIST
#undef SESSION_PHASE
    }

    if (auth_->gameOver()) {
        setPhase(SessionPhase::GameOver);
        return;
    }

    host_.intentDispatcher().flush();

    // update client visual
    client_->present();

    // @TODO: refactor this part
    if (selector_) {
        Vec2i focused = selector_->focus();
        Square square = Square::fromFileRank((uint8_t)focused.x, (uint8_t)focused.y);
        client_->board_view().setHovered(square);
    }
}

void ChessGameSession::tickAwaitPlayerInput() {
    // @TODO: grid adapter should be owned by client/player?
    if (grid_adapter_) {
        grid_adapter_->tick();
    }

    // poll player intents
    auto side = auth_->sideToMove();
    agents_[std::to_underlying(side)]->tick(host_);
}

void ChessGameSession::tickResolvingMove() {
    setPhase(SessionPhase::Animating);
}

void ChessGameSession::tickAnimating() {
    if (isAnimating()) {
        return;
    }

    setPhase(SessionPhase::AwaitPlayerInput);
}

void ChessGameSession::tickGameOver() {
    if (isAnimating()) {
        return;
    }

    LOG_INFO(LogChannel::Game, "Game Over!");

    auto state = std::make_unique<GameOverState>();
    host_.intentDispatcher().queue<ChessStateIntent>(std::move(state));
}

bool ChessGameSession::isAnimating() const {
    auto& query = host_.sceneQuery();
    return query.componentCount(TransformAnimationComponent_Id) != 0;
}

auto ChessGameSession::createPlayer(Color side, PlayerKind kind)
    -> std::unique_ptr<IPlayerAgent> {
    switch (kind) {
        case PlayerKind::LocalHuman:
            return std::make_unique<LocalHumanAgent>(side);
        case PlayerKind::LocalAI:
            return std::make_unique<ChessAIAgent>(side, *client_);
        case PlayerKind::RemoteNetwork:
            return nullptr;
        default:
            return nullptr;
    }
}

// @TODO: this should be configured by MainMenu?
void ChessGameSession::onEnterBoot() {
    MatchConfig config{};
    config.black = { PlayerKind::LocalAI };

    auth_ = std::make_unique<ChessMatchAuthority>(host_);
    client_ = std::make_unique<ChessGameClient>(host_, *this, *auth_);

    const PlayerKind white = config.white.kind;
    const PlayerKind black = config.black.kind;

    agents_[0] = createPlayer(Color::White, white);
    agents_[1] = createPlayer(Color::Black, black);

    const bool any_human = white == PlayerKind::LocalHuman || black == PlayerKind::LocalHuman;
    if (any_human) {
        grid_adapter_ = std::make_unique<ChessGridSelectorAdapter>(
            host_,
            *client_,
            client_->board_view());

        cave::GridSelectController::Callbacks cbs = {
            .can_select = [this](int x, int y) { return grid_adapter_->canSelect(x, y); },
            .on_select = [this](int x, int y) { grid_adapter_->onSelect(x, y); },
            .can_drop = [this](int sx, int sy, int dx, int dy) { return grid_adapter_->canDrop(sx, sy, dx, dy); },
            .on_drop = [this](int sx, int sy, int dx, int dy) { grid_adapter_->onDrop(sx, sy, dx, dy); },
            .on_cancel = [this]() { grid_adapter_->onCancel(); },
            .on_invalid = [this](int sx, int sy, int dx, int dy) { grid_adapter_->onInvalid(sx, sy, dx, dy); }
        };

        selector_ = std::make_unique<cave::GridSelectController>(
            Vec2i(8, 8),
            std::move(cbs));

        grid_adapter_->setController(selector_.get());

        grid_adapter_->setPlayerCb([this](Color side) -> LocalHumanAgent* {
            return dynamic_cast<LocalHumanAgent*>(agents_[std::to_underlying(side)].get());
        });
    }

    client_->onBoot();
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
    if (phase == phase_) {
        return;
    }

#if USING(DEBUG_BUILD)
    auto msg = std::format("SessionPhase {} -> {}", toString(phase_), toString(phase));
    LOG_TRACE(LogChannel::Game, std::move(msg));
#endif

    phase_ = phase;
}

}  // namespace chess
