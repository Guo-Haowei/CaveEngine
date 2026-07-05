#include "ChessGridSelectorAdapter.h"

#include <cassert>

#include "cave/core/math/Plane.h"
#include "cave/game/IHostServices.h"
#include "cave/runtime/controller/GridSelectController.h"
#include "cave/runtime/display/DisplayService.h"
#include "cave/runtime/ecs/components/CameraComponent.h"
#include "cave/runtime/input/IGameInput.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/view/ViewQuery.h"

#include "chess/agents/LocalHumanAgent.h"
#include "chess/game/ChessGameClient.h"
#include "chess/game/ChessIntent.h"
#include "chess/game/ChessMatchAuthority.h"
#include "chess/presentation/ChessBoardView.h"

namespace chess {

using namespace ::cave;
using namespace ::cave::literals;
using namespace ::cave::math;
using namespace ::chess::core;

ChessGridSelectorAdapter::ChessGridSelectorAdapter(IHostServices& host,
                                                   ChessGameClient& game,
                                                   ChessBoardView& board_view) noexcept
    : m_host_(host)
    , m_client(game)
    , m_board_view(board_view) {

    m_camera_id = m_host_.sceneQuery().findFirstByName("game_camera");
    assert(m_camera_id.IsValid());
}

bool ChessGridSelectorAdapter::canSelect(int x, int y) {
    const Square sq = Square::fromFileRank((uint8_t)x, (uint8_t)y);
    std::span<const Move> moves = m_client.legalMoves(sq);

    return !moves.empty();
}

void ChessGridSelectorAdapter::onSelect(int x, int y) {
    const Square sq = Square::fromFileRank((uint8_t)x, (uint8_t)y);
    std::span<const Move> moves = m_client.legalMoves(sq);

    core::Bitboard bb;
    for (Move mv : moves) {
        bb.Set(mv.to());
    }
    m_board_view.setHighlight(bb);
}

bool ChessGridSelectorAdapter::canDrop(int sx, int sy, int dx, int dy) {
    const Square sq = Square::fromFileRank((uint8_t)sx, (uint8_t)sy);

    std::span<const Move> moves = m_client.legalMoves(sq);
    for (Move mv : moves) {
        const auto [from_file, from_rank] = mv.from().fileRank();
        const auto [to_file, to_rank] = mv.to().fileRank();

        if (from_file == sx && from_rank == sy && to_file == dx && to_rank == dy) {
            return true;
        }
    }

    return false;
}

void ChessGridSelectorAdapter::onDrop(int sx, int sy, int dx, int dy) {
    m_board_view.setHighlight({});

    const Position& pos = m_client.replica();
    const Color id = pos.SideToMove();

    if (LocalHumanAgent* agent = m_get_player_func(id)) {
        const Square from = Square::fromFileRank((uint8_t)sx, (uint8_t)sy);
        const Square to = Square::fromFileRank((uint8_t)dx, (uint8_t)dy);

        std::span<const Move> moves = m_client.legalMoves(from);
        Move move = Move::null();
        for (Move mv : moves) {
            if (mv.to() == to) {
                move = mv;
                break;
            }
        }
        assert(move.isValid());

        m_host_.intentDispatcher().queue<ChessMoveIntent>(id, move);
    }
}

void ChessGridSelectorAdapter::onCancel() {
    m_board_view.setHighlight({});
}

void ChessGridSelectorAdapter::onInvalid(int sx, int sy, int dx, int dy) {
    (void)sx;
    (void)sy;
    (void)dx;
    (void)dy;
}

void ChessGridSelectorAdapter::tickPointer(const IGameInput& input) {
    const PointerState& pointer = input.pointerState();

    // @TODO: project ray
    const DisplayService& display = m_host_.displayService();

    const ViewRecord* view = m_host_.viewQuery().resolve(m_host_.viewId());
    if (!view) {
        return;
    }

    Vec2f pos_os = pointer.pos_win + display.windowPos();
    if (!view->display_rect_os.Contains(pos_os.x, pos_os.y)) {
        return;
    }

    Vec2f ndc = view->screenToNDC(pos_os);

    auto camera = (const CameraComponent*)m_host_.sceneQuery().component(CameraComponent_Id, m_camera_id);
    assert(camera);

    Ray ray = Ray::unproject(camera->projectionViewMatrix(), ndc);

    if (!ray.intersects(Plane::xz())) {
        return;
    }

    Vec3f p = ray.hitPoint();
    constexpr Vec3f offset{ -3.5f, 0.0f, -3.5f };
    p -= offset;

    const int file = (int)std::roundf(p.z);
    const int rank = (int)std::roundf(p.x);
    m_controller->focus(file, rank);
    if (input.isJustPressed("ui_accept"_sid)) {
        m_controller->confirm();
    }
    if (input.isJustPressed("ui_back"_sid)) {
        m_controller->cancel();
    }
}

void ChessGridSelectorAdapter::tickKeyboard(const IGameInput& input) {
    if (input.isJustPressed("ui_right"_sid)) {
        m_controller->moveFocus(1, 0);
    }
    if (input.isJustPressed("ui_left"_sid)) {
        m_controller->moveFocus(-1, 0);
    }
    if (input.isJustPressed("ui_up"_sid)) {
        m_controller->moveFocus(0, 1);
    }
    if (input.isJustPressed("ui_down"_sid)) {
        m_controller->moveFocus(0, -1);
    }
    if (input.isJustPressed("ui_accept"_sid)) {
        m_controller->confirm();
    }
    if (input.isJustPressed("ui_back"_sid)) {
        m_controller->cancel();
    }

    const float dx = input.getStrength("ui_axis_x"_sid);
    const float dy = input.getStrength("ui_axis_y"_sid);
    if (dx > 0.5f) {
        m_controller->moveFocus(1, 0);
    } else if (dx < -0.5f) {
        m_controller->moveFocus(-1, 0);
    }

    if (dy > 0.5f) {
        m_controller->moveFocus(0, 1);
    } else if (dy < -0.5f) {
        m_controller->moveFocus(0, -1);
    }
}

void ChessGridSelectorAdapter::tick() {
    const IGameInput& input = m_host_.gameInput();
    tickPointer(input);
    tickKeyboard(input);
}

}  // namespace chess
