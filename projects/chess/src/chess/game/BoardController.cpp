#include "BoardController.h"

#include "cave/runtime/scene/SceneCommandWriter.h"

#include "chess/core/Bitboard.h"
#include "chess/core/Piece.h"
#include "chess/presentation/ChessViewFactory.h"

namespace chess {

using ::cave::ecs::Entity;
using namespace ::cave;
using namespace ::cave::literals;
using namespace ::cave::math;
using namespace ::chess::core;

void SpawnObjects(SceneQuery& query, SceneCommandWriter& writer) {
    // @TODO: use FEN instead
    constexpr std::array<std::array<Piece, 8>, 8> kInitialBoard = { {
        { Piece::WR, Piece::WN, Piece::WB, Piece::WQ, Piece::WK, Piece::WB, Piece::WN, Piece::WR },
        { Piece::WP, Piece::WP, Piece::WP, Piece::WP, Piece::WP, Piece::WP, Piece::WP, Piece::WP },
        { Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null },
        { Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null },
        { Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null },
        { Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null, Piece::Null },
        { Piece::BP, Piece::BP, Piece::BP, Piece::BP, Piece::BP, Piece::BP, Piece::BP, Piece::BP },
        { Piece::BR, Piece::BN, Piece::BB, Piece::BQ, Piece::BK, Piece::BB, Piece::BN, Piece::BR },
    } };

    using chess::Piece;
    using ecs::Entity;

    Entity offset_node = query.findFirstByName("transform");
    DEV_ASSERT(offset_node.valid());

    writer.setNoSave(true);

    Entity piece_parent = writer.transformObject("pieces");
    Entity tile_parent = writer.transformObject("tiles");

    writer.attachChild(piece_parent, offset_node);
    writer.attachChild(tile_parent, offset_node);

    chess::ChessViewFactory factory(writer, piece_parent);

    // Create regular pieces
    for (uint8_t rank = 0; rank < 8; ++rank) {
        for (uint8_t file = 0; file < 8; ++file) {
            Square square = Square::fromFileRank(file, rank);
            const Piece p = kInitialBoard[rank][file];
            if (p == Piece::Null) continue;
            factory.createPiece(square, p);
        }
    }

    // Create selector
    factory.createTile(Square::A1, {
                                       Vec4f(1.0f, 0.0f, 0.0f, 0.5f),
                                       "grid_selector",
                                       offset_node,
                                   });

    factory.setVisible(false);

    // Create extra pieces for promotion
    // @NOTE: only supports promote to queen
    const Piece extra_pieces[] = {
        // Piece::WN,
        // Piece::WB,
        // Piece::WR,
        Piece::WQ,
        // Piece::BN,
        // Piece::BB,
        // Piece::BR,
        Piece::BQ,
    };

    for (Piece piece : extra_pieces) {
        for (int i = 0; i < 8; ++i) {
            factory.createPiece(Square::A1, piece);
        }
    }

    // Create tiles
    for (uint8_t i = 0; i < 64; ++i) {
        factory.createTile(Square(i), {
                                          Vec4f(0.0f, 1.0f, 0.0f, 0.5f),
                                          nullptr,
                                          tile_parent,
                                      });
    }
}

BoardController::BoardController() = default;
BoardController::~BoardController() = default;

void BoardController::alwaysRun(SceneCommandWriter& writer) {
    SpawnObjects(query(), writer);
}

void BoardController::start() {
    m_game = std::make_unique<ChessGameMode>(runtime(), m_intent_bus);
    m_game->onEnter();
}

void BoardController::destroy() {
    m_game->onExit();
    m_game.reset();
}

void BoardController::update(float dt) {
    m_game->tick(dt);
}

}  // namespace chess
