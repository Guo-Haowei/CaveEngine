#include "ChessBoardView.h"

#include "cave/core/error/ErrorMacros.h"
#include "cave/runtime/scene/SceneQuery.h"
#include "cave/runtime/ecs/components/MeshRendererComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

#include "chess/presentation/ChessUtils.h"

namespace chess {

using namespace ::cave;
using namespace ::chess::core;

void ChessBoardView::initialize(SceneQuery& query) {
    selector_ = query.findFirstByName("grid_selector");

    for (uint8_t i = 0; i < 64; ++i) {
        const char* name = Square(i).uci();
        tiles_[i] = query.findFirstByName(name);
    }
}

void ChessBoardView::drawBoard(SceneQuery& query) {
    for (uint8_t i = 0; i < 64; ++i) {
        const Square sq(i);
        bool visible = highlights_.Test(sq);
        if (sq == hovered_square_) {
            visible = false;
        }
        const ecs::Entity tile = tiles_[i];

        auto renderer = query.component<MeshRendererComponent>(tile);
        if (DEV_VERIFY(renderer)) {
            renderer->SetVisible(visible);
        }
    }

    auto transform = query.component<TransformComponent>(selector_);
    if (DEV_VERIFY(transform)) {
        transform->setTranslation(squareToVec(hovered_square_));
    }
}

}  // namespace chess
