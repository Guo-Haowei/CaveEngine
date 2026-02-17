#pragma once
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

enum class HighlightHint {
    LegalMove,
    Selected,
};

class ChessPresenter {
    using Entity = cave::ecs::Entity;

public:
    void Present(const PresentationContext& p_ctx);

    void OnGameBegin(cave::SceneQuery& p_query);
    void OnGameEnd();

    void HighlightSquare(core::Square p_sq,
                         HighlightHint p_hint = HighlightHint::LegalMove);

private:
    Entity m_selector;
};

}  // namespace chess
