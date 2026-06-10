#include "ChessBoardView.h"

#include "cave/game/IHostServices.h"
#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneQuery.h"

namespace chess {

using namespace ::cave;
using namespace ::cave::literals;
using namespace ::cave::math;
using namespace ::chess::core;

static constexpr StringId kVisibility = "visibility"_sid;
static constexpr StringId kTranslationId = "translation"_sid;

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

// @TODO: refactor this
static inline Vector3f squareToVec(Square square) {
    const auto [file, rank] = square.fileRank();
    return Vector3f{ (float)rank, 0.0f, (float)file };
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
