#include "cave/runtime/tile_map/TileData.h"

namespace cave {

namespace {

TileCell MakeTileCell(uint16_t tile_id) {
    return TileCell{
        .tile_id = TileId(tile_id),
        .terrain_id = TerrainId::null(),
    };
}

TileCell MakeTerrainCell(uint16_t terrain_id) {
    return TileCell{
        .tile_id = TileId::null(),
        .terrain_id = TerrainId(terrain_id),
    };
}

}  // namespace

// =============================================================================
// TileCell
// =============================================================================

TEST(TileCell, DefaultConstructedCellIsEmpty) {
    const TileCell cell;

    EXPECT_TRUE(cell.empty());
    EXPECT_FALSE(cell.hasTile());
    EXPECT_FALSE(cell.hasTerrain());

    EXPECT_EQ(cell.tile_id, TileId::null());
    EXPECT_EQ(cell.terrain_id, TerrainId::null());
}

TEST(TileCell, TileOnlyCellIsNotEmpty) {
    const TileCell cell = MakeTileCell(42);

    EXPECT_FALSE(cell.empty());
    EXPECT_TRUE(cell.hasTile());
    EXPECT_FALSE(cell.hasTerrain());

    EXPECT_EQ(cell.tile_id, TileId(42));
    EXPECT_EQ(cell.terrain_id, TerrainId::null());
}

TEST(TileCell, TerrainOnlyCellIsNotEmpty) {
    const TileCell cell = MakeTerrainCell(3);

    EXPECT_FALSE(cell.empty());
    EXPECT_FALSE(cell.hasTile());
    EXPECT_TRUE(cell.hasTerrain());

    EXPECT_EQ(cell.tile_id, TileId::null());
    EXPECT_EQ(cell.terrain_id, TerrainId(3));
}

TEST(TileCell, ClearResetsBothIds) {
    TileCell cell{
        .tile_id = TileId(42),
        .terrain_id = TerrainId(3),
    };

    cell.clear();

    EXPECT_TRUE(cell.empty());
    EXPECT_FALSE(cell.hasTile());
    EXPECT_FALSE(cell.hasTerrain());

    EXPECT_EQ(cell.tile_id, TileId::null());
    EXPECT_EQ(cell.terrain_id, TerrainId::null());
}

// =============================================================================
// TileChunk
// =============================================================================

TEST(TileChunk, DefaultConstructedChunkIsEmpty) {
    TileChunk chunk;

    EXPECT_TRUE(chunk.empty());

    for (int16_t y = 0; y < kTileChunkSize; ++y) {
        for (int16_t x = 0; x < kTileChunkSize; ++x) {
            const TileCell& cell = chunk.at(x, y);

            EXPECT_TRUE(cell.empty());
            EXPECT_FALSE(cell.hasTile());
            EXPECT_FALSE(cell.hasTerrain());

            EXPECT_EQ(cell.tile_id, TileId::null());
            EXPECT_EQ(cell.terrain_id, TerrainId::null());
        }
    }
}

TEST(TileChunk, AtReadsAndWritesCell) {
    TileChunk chunk;

    const TileCell expected{
        .tile_id = TileId(42),
        .terrain_id = TerrainId(7),
    };

    chunk.at(3, 5) = expected;

    EXPECT_EQ(chunk.at(3, 5), expected);
    EXPECT_FALSE(chunk.empty());

    EXPECT_TRUE(chunk.at(0, 0).empty());
    EXPECT_TRUE(chunk.at(31, 31).empty());
}

TEST(TileChunk, WritingTerrainOnlyCellMakesChunkNonEmpty) {
    TileChunk chunk;

    const TileCell expected = MakeTerrainCell(2);
    chunk.at(3, 5) = expected;

    EXPECT_EQ(chunk.at(3, 5), expected);
    EXPECT_FALSE(chunk.empty());

    EXPECT_FALSE(chunk.at(3, 5).hasTile());
    EXPECT_TRUE(chunk.at(3, 5).hasTerrain());
}

// =============================================================================
// Tile coordinates
// =============================================================================

TEST(TileCoord, PositiveCoordToChunkAndLocal) {
    const TileCoord coord{ 33, 65 };

    EXPECT_EQ(ToTileChunkCoord(coord), (TileChunkCoord{ 1, 2 }));

    EXPECT_EQ(ToTileLocalX(coord), 1);
    EXPECT_EQ(ToTileLocalY(coord), 1);
}

TEST(TileCoord, ZeroCoordToChunkAndLocal) {
    const TileCoord coord{ 0, 0 };

    EXPECT_EQ(ToTileChunkCoord(coord), (TileChunkCoord{ 0, 0 }));

    EXPECT_EQ(ToTileLocalX(coord), 0);
    EXPECT_EQ(ToTileLocalY(coord), 0);
}

TEST(TileCoord, PositiveChunkBorder) {
    EXPECT_EQ(ToTileChunkCoord(TileCoord{ 31, 31 }), (TileChunkCoord{ 0, 0 }));
    EXPECT_EQ(ToTileChunkCoord(TileCoord{ 32, 32 }), (TileChunkCoord{ 1, 1 }));

    EXPECT_EQ(ToTileLocalX(TileCoord{ 31, 31 }), 31);
    EXPECT_EQ(ToTileLocalY(TileCoord{ 31, 31 }), 31);

    EXPECT_EQ(ToTileLocalX(TileCoord{ 32, 32 }), 0);
    EXPECT_EQ(ToTileLocalY(TileCoord{ 32, 32 }), 0);
}

TEST(TileCoord, NegativeCoordToChunkAndLocal) {
    const TileCoord coord{ -1, -1 };

    EXPECT_EQ(ToTileChunkCoord(coord), (TileChunkCoord{ -1, -1 }));

    EXPECT_EQ(ToTileLocalX(coord), 31);
    EXPECT_EQ(ToTileLocalY(coord), 31);
}

TEST(TileCoord, NegativeChunkBorder) {
    EXPECT_EQ(ToTileChunkCoord(TileCoord{ -32, -32 }), (TileChunkCoord{ -1, -1 }));

    EXPECT_EQ(ToTileLocalX(TileCoord{ -32, -32 }), 0);

    EXPECT_EQ(ToTileLocalY(TileCoord{ -32, -32 }), 0);

    EXPECT_EQ(ToTileChunkCoord(TileCoord{ -33, -33 }), (TileChunkCoord{ -2, -2 }));

    EXPECT_EQ(ToTileLocalX(TileCoord{ -33, -33 }), 31);

    EXPECT_EQ(ToTileLocalY(TileCoord{ -33, -33 }), 31);
}

TEST(TileCoord, ChunkLocalToTileCoord) {
    EXPECT_EQ(ToTileCoord(TileChunkCoord{ 0, 0 }, 0, 0), (TileCoord{ 0, 0 }));

    EXPECT_EQ(ToTileCoord(TileChunkCoord{ 0, 0 }, 31, 31), (TileCoord{ 31, 31 }));

    EXPECT_EQ(ToTileCoord(TileChunkCoord{ 1, 2 }, 1, 1), (TileCoord{ 33, 65 }));

    EXPECT_EQ(ToTileCoord(TileChunkCoord{ -1, -1 }, 31, 31), (TileCoord{ -1, -1 }));

    EXPECT_EQ(ToTileCoord(TileChunkCoord{ -1, -1 }, 0, 0), (TileCoord{ -32, -32 }));
}

TEST(TileCoord, TileCoordRoundTripsThroughChunkAndLocal) {
    constexpr TileCoord coords[] = {
        TileCoord{ 0, 0 },
        TileCoord{ 31, 31 },
        TileCoord{ 32, 32 },
        TileCoord{ 33, 65 },
        TileCoord{ -1, -1 },
        TileCoord{ -32, -32 },
        TileCoord{ -33, -33 },
        TileCoord{ 64, -65 },
    };

    for (const TileCoord coord : coords) {
        const TileChunkCoord chunk_coord = ToTileChunkCoord(coord);
        const int16_t local_x = ToTileLocalX(coord);
        const int16_t local_y = ToTileLocalY(coord);

        EXPECT_GE(local_x, 0);
        EXPECT_GE(local_y, 0);
        EXPECT_LT(local_x, kTileChunkSize);
        EXPECT_LT(local_y, kTileChunkSize);

        EXPECT_EQ(ToTileCoord(chunk_coord, local_x, local_y), coord);
    }
}

// =============================================================================
// ChunkedTileData
// =============================================================================

TEST(ChunkedTileData, NewDataHasNoCells) {
    ChunkedTileData data;

    EXPECT_TRUE(data.chunks().empty());
    EXPECT_TRUE(data.cellAt(TileCoord{ 0, 0 }).is_none());
    EXPECT_TRUE(data.cellAt(TileCoord{ -1, -1 }).is_none());
}

TEST(ChunkedTileData, AddTileCreatesChunk) {
    ChunkedTileData data;

    const TileCell expected = MakeTileCell(7);

    EXPECT_TRUE(data.addCell(TileCoord{ 0, 0 }, expected));
    const Option<TileCell> cell = data.cellAt(TileCoord{ 0, 0 });

    ASSERT_TRUE(cell.is_some());
    EXPECT_EQ(cell.unwrap_unchecked(), expected);

    EXPECT_EQ(data.chunks().size(), 1u);
    EXPECT_TRUE(data.chunks().contains(TileChunkCoord{ 0, 0 }));
}

TEST(ChunkedTileData, AddTileLeavesOtherChunkCellsEmpty) {
    ChunkedTileData data;

    const TileCoord painted_coord{ 3, 5 };
    const TileCell painted_cell = MakeTileCell(7);

    ASSERT_TRUE(data.addCell(painted_coord, painted_cell));

    ASSERT_EQ(data.chunks().size(), 1u);

    const auto chunk_it = data.chunks().find(TileChunkCoord{ 0, 0 });
    ASSERT_NE(chunk_it, data.chunks().end());
    ASSERT_TRUE(chunk_it->second != nullptr);

    const TileChunk& chunk = *chunk_it->second;

    for (int16_t y = 0; y < kTileChunkSize; ++y) {
        for (int16_t x = 0; x < kTileChunkSize; ++x) {
            const TileCell& cell = chunk.at(x, y);

            if (x == painted_coord.x && y == painted_coord.y) {
                EXPECT_EQ(cell, painted_cell);
                continue;
            }

            EXPECT_TRUE(cell.empty())
                << "Unexpected occupied cell at local coordinate ("
                << x << ", " << y << ')';

            EXPECT_EQ(cell.tile_id, TileId::null());
            EXPECT_EQ(cell.terrain_id, TerrainId::null());
        }
    }
}

TEST(ChunkedTileData, AddSameCellReturnsFalse) {
    ChunkedTileData data;

    const TileCell cell = MakeTileCell(7);

    EXPECT_TRUE(data.addCell(TileCoord{ 0, 0 }, cell));

    EXPECT_FALSE(data.addCell(TileCoord{ 0, 0 }, cell));

    const Option<TileCell> stored = data.cellAt(TileCoord{ 0, 0 });

    ASSERT_TRUE(stored.is_some());
    EXPECT_EQ(stored.unwrap_unchecked(), cell);

    EXPECT_EQ(data.chunks().size(), 1u);
}

TEST(ChunkedTileData, AddDifferentCellReplacesOldCell) {
    ChunkedTileData data;

    const TileCell first = MakeTileCell(7);
    const TileCell second{
        .tile_id = TileId(9),
        .terrain_id = TerrainId(2),
    };

    EXPECT_TRUE(data.addCell(TileCoord{ 0, 0 }, first));

    EXPECT_TRUE(data.addCell(TileCoord{ 0, 0 }, second));

    const Option<TileCell> stored = data.cellAt(TileCoord{ 0, 0 });

    ASSERT_TRUE(stored.is_some());
    EXPECT_EQ(stored.unwrap_unchecked(), second);

    EXPECT_EQ(data.chunks().size(), 1u);
}

TEST(ChunkedTileData, CanStoreTerrainOnlyCell) {
    ChunkedTileData data;

    const TileCell expected = MakeTerrainCell(0);

    EXPECT_TRUE(data.addCell(TileCoord{ 4, 7 }, expected));
    const Option<TileCell> stored = data.cellAt(TileCoord{ 4, 7 });

    ASSERT_TRUE(stored.is_some());

    EXPECT_EQ(stored.unwrap_unchecked(), expected);
    EXPECT_FALSE(stored.unwrap_unchecked().hasTile());
    EXPECT_TRUE(stored.unwrap_unchecked().hasTerrain());
}

TEST(ChunkedTileData, CanStoreTileAndTerrainTogether) {
    ChunkedTileData data;

    const TileCell expected{
        .tile_id = TileId(7),
        .terrain_id = TerrainId(0),
    };

    EXPECT_TRUE(data.addCell(TileCoord{ 4, 7 }, expected));

    const Option<TileCell> stored = data.cellAt(TileCoord{ 4, 7 });

    ASSERT_TRUE(stored.is_some());
    EXPECT_EQ(stored.unwrap_unchecked(), expected);
}

TEST(ChunkedTileData, RemoveTileRemovesCell) {
    ChunkedTileData data;

    EXPECT_TRUE(data.addCell(TileCoord{ 0, 0 }, MakeTileCell(7)));
    EXPECT_TRUE(data.removeCell(TileCoord{ 0, 0 }));
    EXPECT_TRUE(data.cellAt(TileCoord{ 0, 0 }).is_none());
}

TEST(ChunkedTileData, RemoveTerrainOnlyCell) {
    ChunkedTileData data;

    EXPECT_TRUE(data.addCell(TileCoord{ 0, 0 }, MakeTerrainCell(0)));
    EXPECT_TRUE(data.removeCell(TileCoord{ 0, 0 }));
    EXPECT_TRUE(data.cellAt(TileCoord{ 0, 0 }).is_none());
}

TEST(ChunkedTileData, RemoveMissingTileReturnsFalse) {
    ChunkedTileData data;

    EXPECT_FALSE(data.removeCell(TileCoord{ 0, 0 }));
    EXPECT_TRUE(data.addCell(TileCoord{ 0, 0 }, MakeTileCell(7)));
    EXPECT_FALSE(data.removeCell(TileCoord{ 1, 0 }));
}

TEST(ChunkedTileData, RemoveOneCellKeepsNonEmptyChunk) {
    ChunkedTileData data;

    EXPECT_TRUE(data.addCell(TileCoord{ 0, 0 }, MakeTileCell(7)));
    EXPECT_TRUE(data.addCell(TileCoord{ 1, 0 }, MakeTileCell(8)));

    EXPECT_TRUE(data.removeCell(TileCoord{ 0, 0 }));
    EXPECT_EQ(data.chunks().size(), 1u);

    EXPECT_TRUE(data.cellAt(TileCoord{ 0, 0 }).is_none());

    const Option<TileCell> cell = data.cellAt(TileCoord{ 1, 0 });

    ASSERT_TRUE(cell.is_some());
    EXPECT_EQ(cell.unwrap_unchecked(), MakeTileCell(8));
}

TEST(ChunkedTileData, NegativeTileRoundTrip) {
    ChunkedTileData data;

    const TileCell expected = MakeTileCell(11);

    EXPECT_TRUE(data.addCell(TileCoord{ -1, -1 }, expected));

    const Option<TileCell> stored = data.cellAt(TileCoord{ -1, -1 });

    ASSERT_TRUE(stored.is_some());
    EXPECT_EQ(stored.unwrap_unchecked(), expected);

    EXPECT_EQ(data.chunks().size(), 1u);

    EXPECT_TRUE(data.chunks().contains(TileChunkCoord{ -1, -1 }));
}

TEST(ChunkedTileData, DifferentChunksAreIndependent) {
    ChunkedTileData data;

    const TileCell first = MakeTileCell(1);
    const TileCell second = MakeTileCell(2);
    const TileCell third = MakeTileCell(3);

    EXPECT_TRUE(data.addCell(TileCoord{ 0, 0 }, first));

    EXPECT_TRUE(data.addCell(TileCoord{ 32, 0 }, second));

    EXPECT_TRUE(data.addCell(TileCoord{ -1, 0 }, third));

    EXPECT_EQ(data.chunks().size(), 3u);

    const Option<TileCell> first_stored = data.cellAt(TileCoord{ 0, 0 });
    const Option<TileCell> second_stored = data.cellAt(TileCoord{ 32, 0 });
    const Option<TileCell> third_stored = data.cellAt(TileCoord{ -1, 0 });

    ASSERT_TRUE(first_stored.is_some());
    ASSERT_TRUE(second_stored.is_some());
    ASSERT_TRUE(third_stored.is_some());

    EXPECT_EQ(first_stored.unwrap_unchecked(), first);
    EXPECT_EQ(second_stored.unwrap_unchecked(), second);
    EXPECT_EQ(third_stored.unwrap_unchecked(), third);
}

TEST(ChunkedTileData, RemoveLastTileErasesEmptyChunk) {
    ChunkedTileData data;

    EXPECT_TRUE(data.addCell(TileCoord{ 0, 0 }, MakeTileCell(7)));

    ASSERT_EQ(data.chunks().size(), 1u);

    EXPECT_TRUE(data.removeCell(TileCoord{ 0, 0 }));
}

#if GTEST_HAS_DEATH_TEST

TEST(ChunkedTileData, AddingEmptyCellShouldNotBeAllowed) {
    ChunkedTileData data;

    EXPECT_DEATH(data.addCell(TileCoord{ 0, 0 }, TileCell{}), "");
}

#endif

}  // namespace cave
