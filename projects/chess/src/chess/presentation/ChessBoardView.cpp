#include "ChessBoardView.h"

#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"

#include "chess/presentation/ChessUtils.h"

namespace chess {

using namespace ::cave;
using namespace ::chess::core;

ChessBoardView::ChessBoardView(IHostServices& host) noexcept
    : host_(host)
    , writer_(host.sceneWriter()) {
}

void ChessBoardView::initialize() {
    auto& query = host_.sceneQuery();
    selector_ = query.findFirstByName("grid_selector");

    // tiles
    for (uint8_t i = 0; i < 64; ++i) {
        const char* name = Square(i).uci();
        tiles_[i] = query.findFirstByName(name);
    }
}

void ChessBoardView::drawBoard() {
    for (uint8_t i = 0; i < 64; ++i) {
        const Square sq(i);
        bool visible = highlights_.Test(sq);
        if (sq == hovered_square_) {
            visible = false;
        }
        const ecs::Entity tile = tiles_[i];
        writer_.SetProperty(tile,
                            cave::MeshRendererComponent_Id,
                            kVisibility,
                            visible);
    }

    Vector3f position = squareToVec(hovered_square_);
    writer_.SetProperty(selector_,
                        cave::TransformComponent_Id,
                        kTranslationId,
                        position);
}

}  // namespace chess
