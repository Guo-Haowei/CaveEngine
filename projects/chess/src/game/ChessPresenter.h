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
    core::Square selected;
};

class ChessPresenter {
    using Entity = cave::ecs::Entity;

public:
    void Present(const PresentationContext& p_ctx);

    void OnGameBegin(cave::SceneQuery& p_query);
    void OnGameEnd();

    void HighlightSquares(core::Bitboard p_bb);

private:
    Entity m_selector;

    std::array<Entity, 64> m_tiles;

    core::Bitboard m_highlights;
};

}  // namespace chess
