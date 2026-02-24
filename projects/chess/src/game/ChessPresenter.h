#pragma once
#include <array>
#include "cave/runtime/ecs/Entity.h"

#include "core/Position.h"

// clang-format off
namespace cave { class SceneQuery; }
namespace cave { class IHostServices; }
// clang-format on

namespace chess {

struct PresentationContext {
    cave::IHostServices& host;
};

class ChessPresenter {
    using Entity = cave::ecs::Entity;

public:
    void Present(const PresentationContext& p_ctx);
    void RedrawPosition(cave::IHostServices& p_host, const core::Position& p_position);

    void OnBoot(cave::SceneQuery& p_query);

    void SetFocusedSquare(core::Square p_sq) {
        m_focused = p_sq;
    }

    void SetHighlightSquares(core::Bitboard p_bb) {
        m_highlights = p_bb;
    }

private:
    Entity m_selector;

    std::array<Entity, 64> m_tiles;
    std::array<std::vector<Entity>, core::kPieceMax> m_piece_pools;

    core::Square m_focused{ 0 };
    core::Bitboard m_highlights;
};

}  // namespace chess
