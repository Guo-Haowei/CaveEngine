#include "ChessBoardView.h"

#include "cave/core/error/ErrorMacros.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/render/components/MeshRendererComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

#include "chess/presentation/ChessUtils.h"

namespace chess {

using namespace ::cave;
using namespace ::chess::core;

void ChessBoardView::initialize() {
    m_selector = m_query.findFirstByName("grid_selector");

    for (uint8_t i = 0; i < 64; ++i) {
        const char* name = Square(i).uci();
        m_tiles[i] = m_query.findFirstByName(name);
    }
}

void ChessBoardView::drawBoard() {
    for (uint8_t i = 0; i < 64; ++i) {
        const Square sq(i);
        bool visible = m_highlights.Test(sq);
        if (sq == m_hovered_square) {
            visible = false;
        }
        const ecs::Entity tile = m_tiles[i];

        auto renderer = m_query.component<MeshRendererComponent>(tile);
        if (DEV_VERIFY(renderer)) {
            renderer->setVisible(visible);
        }
    }

    auto transform = m_query.component<TransformComponent>(m_selector);
    if (DEV_VERIFY(transform)) {
        transform->setTranslation(squareToVec(m_hovered_square));
    }
}

}  // namespace chess
