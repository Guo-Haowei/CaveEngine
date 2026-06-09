#pragma once
#include "cave/core/math/Vector.h"
#include "cave/runtime/ecs/Entity.h"

#include "chess/core/Piece.h"
#include "chess/core/Square.h"

// clang-format off
namespace cave { class SceneCommandWriter; }
// clang-format on

namespace chess {

// @TODO: factory and registry

class ChessViewFactory {
    using Entity = ::cave::ecs::Entity;

public:
    ChessViewFactory(cave::SceneCommandWriter& writer, Entity parent);

    Entity createPiece(core::Square square, core::Piece piece);

    // @TODO: move tile creation to somewhere else
    struct TileInitInfo {
        cave::math::Vector4f color;
        const char* name;
        Entity parent;
    };
    Entity createTile(core::Square square, const TileInitInfo& info);

    void setVisible(bool visible) { visible_ = visible; }

private:
    cave::SceneCommandWriter& writer_;
    Entity parent_;
    const char* materials_[2];

    std::array<uint8_t, core::kPieceMax> piece_counters_{ 0 };

    bool visible_{ true };
};

}  // namespace chess
