#pragma once
#include <array>
#include "cave/runtime/ecs/Entity.h"

#include "core/Position.h"

// clang-format off
namespace cave { class IHostServices; }
namespace cave { class SceneCommandWriter; }
namespace cave { class SceneQuery; }
// clang-format on

namespace chess {

class ChessPresenter {
    using Entity = cave::ecs::Entity;

public:
    ChessPresenter(cave::IHostServices& p_host) noexcept
        : m_host(p_host) {
    }

    void Present();

    // ==== Board Representation ====
    void OnBoot(cave::SceneQuery& p_query);

    void InitBoard(const core::Position& p_position);

    void ApplyMove(core::Move p_mv);

    Entity GetEntityAt(core::Square p_sq) const {
        return m_board[p_sq.Index()];
    }

    // ==== Grid ====
    void SetFocusedSquare(core::Square p_sq) {
        m_focused = p_sq;
    }

    void SetHighlightSquares(core::Bitboard p_bb) {
        m_highlights = p_bb;
    }

private:
    void SetEntityAt(cave::SceneCommandWriter& p_writer,
                     core::Square p_sq,
                     Entity p_ent);

    void ClearSquare(cave::SceneCommandWriter& p_writer,
                     core::Square p_sq);

    cave::IHostServices& m_host;
    Entity m_selector;

    std::array<Entity, 64> m_tiles;
    std::array<std::vector<Entity>, core::kPieceMax> m_piece_pools;

    core::Square m_focused{ 0 };
    core::Bitboard m_highlights;

    std::array<Entity, 64> m_board;
};

}  // namespace chess
