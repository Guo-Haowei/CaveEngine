#include "ChessSpawner.h"

#include "cave/core/error/ErrorMacros.h"

#include "chess/core/Bitboard.h"
#include "chess/core/Piece.h"
#include "chess/presentation/ChessViewFactory.h"

namespace chess {

using ::cave::ecs::Entity;
using namespace ::cave;
using namespace ::cave::literals;
using namespace ::cave::math;
using namespace ::chess::core;

Spawner::Spawner(SpawnType type, SceneQuery& query, SceneCommandWriter& writer)
    : m_type(type)
    , m_query(query)
    , m_writer(writer) {

    m_prev_no_save = m_writer.noSave();
    m_writer.setNoSave(true);

    m_offset_node = m_query.findFirstByName("transform");
    DEV_ASSERT(m_offset_node.valid());

    m_factory = MakeOwner<ChessViewFactory>(m_writer);
}

Spawner::~Spawner() {
    m_writer.setNoSave(m_prev_no_save);
}

void Spawner::spawnPieces() {
    Entity piece_parent = m_writer.transformObject("pieces");
    m_writer.attachChild(piece_parent, m_offset_node);

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

    // Create regular pieces
    for (uint8_t rank = 0; rank < 8; ++rank) {
        for (uint8_t file = 0; file < 8; ++file) {
            Square square = Square::fromFileRank(file, rank);
            const Piece p = kInitialBoard[rank][file];
            if (p == Piece::Null) continue;
            m_factory->createPiece(square,
                                   {
                                       .piece = p,
                                       .parent = piece_parent,
                                   });
        }
    }

    if (m_type == SpawnType::Gameplay) {
        spawnExtraPieces(piece_parent);
        spawnTiles();
    }
}

void Spawner::spawnExtraPieces(Entity piece_parent) {
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
            m_factory->createPiece(Square::A1,
                                   {
                                       .piece = piece,
                                       .parent = piece_parent,
                                       .visible = false,
                                   });
        }
    }
}

void Spawner::spawnTiles() {
    Entity tile_parent = m_writer.transformObject("tiles");
    m_writer.attachChild(tile_parent, m_offset_node);

    // Create selector
    m_factory->createTile(Square::A1,
                          {
                              Vec4f(1.0f, 0.0f, 0.0f, 0.5f),
                              "grid_selector",
                              m_offset_node,
                              true,
                          });

    // Create tiles
    for (uint8_t i = 0; i < 64; ++i) {
        m_factory->createTile(Square(i), {
                                             Vec4f(0.0f, 1.0f, 0.0f, 0.5f),
                                             nullptr,
                                             tile_parent,
                                         });
    }
}

}  // namespace chess
