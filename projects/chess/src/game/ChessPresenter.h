#pragma once
#include <array>
#include "cave/runtime/ecs/Entity.h"

#include "core/Bitboard.h"

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

    void OnGameBegin(cave::SceneQuery& p_query);
    void OnGameEnd();

    void SetSelectedSquare(core::Square p_sq) {
        m_selected = p_sq;
    }

    void SetHighlightSquares(core::Bitboard p_bb) {
        m_highlights = p_bb;
    }

private:
    Entity m_selector;

    std::array<Entity, 64> m_tiles;

    core::Square m_selected{ 0 };
    core::Bitboard m_highlights;
};

}  // namespace chess
