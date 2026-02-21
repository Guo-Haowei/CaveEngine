#include "ChessGameClient.h"

#include "cave/game/IHostServices.h"
#include "cave/runtime/framework/IInputService.h"

#include "core/MoveGen.h"

namespace chess {

using cave::math::Vector2i;
using core::Move;
using core::MoveGen;
using core::Position;

ChessGameClient::ChessGameClient()
    : m_presenter{}
    , m_grid_adapter{ *this, m_presenter } {

    cave::GridSelectController::Callbacks cbs = {
        .can_select = [this](int x, int y) { return m_grid_adapter.CanSelect(x, y); },
        .on_select = [this](int x, int y) { m_grid_adapter.OnSelect(x, y); },
        .can_drop = [this](int sx, int sy, int dx, int dy) { return m_grid_adapter.CanDrop(sx, sy, dx, dy); },
        .on_drop = [this](int sx, int sy, int dx, int dy) { m_grid_adapter.OnDrop(sx, sy, dx, dy); },
        .on_cancel = [this]() { m_grid_adapter.OnCancel(); },
        .on_invalid = [this](int sx, int sy, int dx, int dy) { m_grid_adapter.OnInvalid(sx, sy, dx, dy); }
    };

    m_selector = std::make_unique<cave::GridSelectController>(
        Vector2i(8, 8),
        std::move(cbs));
}

void ChessGameClient::ProcessInput(cave::IInputService& p_input) {
    using cave::StringId;
    using cave::math::Vector2i;

    if (p_input.IsActionJustPressed(StringId("ui_right"))) {
        m_selector->MoveFocus(Vector2i(1, 0));
    }
    if (p_input.IsActionJustPressed(StringId("ui_left"))) {
        m_selector->MoveFocus(Vector2i(-1, 0));
    }
    if (p_input.IsActionJustPressed(StringId("ui_up"))) {
        m_selector->MoveFocus(Vector2i(0, 1));
    }
    if (p_input.IsActionJustPressed(StringId("ui_down"))) {
        m_selector->MoveFocus(Vector2i(0, -1));
    }
    if (p_input.IsActionJustPressed(StringId("ui_accept"))) {
        m_selector->Confirm();
    }
    if (p_input.IsActionJustPressed(StringId("ui_back"))) {
        m_selector->Cancel();
    }
}

void ChessGameClient::ResetBoard() {
    m_replicated = Position::Default();

    OnPositionChange();
}

void ChessGameClient::OnGameBegin(cave::IHostServices& p_host) {
    m_presenter.OnGameBegin(p_host.SceneQuery());
    ResetBoard();
}

void ChessGameClient::OnGameEnd(cave::IHostServices& p_host) {
    unused(p_host);
    m_presenter.OnGameEnd();
}

void ChessGameClient::Tick(cave::IHostServices& p_host) {
    ProcessInput(p_host.Input());

    const Vector2i focused = m_selector->GetFocused();

    PresentationContext ctx{
        .host = p_host,
        .selected = core::Square::FromFileRank((uint8_t)focused.x, (uint8_t)focused.y),
    };

    m_presenter.Present(ctx);
}

void ChessGameClient::OnPositionChange() {
    MoveGen::Pseudo(m_replicated, m_moves);

    m_move_cache.clear();
    for (Move mv : m_moves) {
        m_move_cache[mv.from].push_back(mv);
    }
}

std::span<const core::Move> ChessGameClient::LegalMovesFromSquare(core::Square p_sq) {
    auto it = m_move_cache.find(p_sq);
    if (it == m_move_cache.end()) {
        return {};
    }

    return it->second;
}

}  // namespace chess
