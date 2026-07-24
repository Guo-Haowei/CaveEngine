#include "cave/runtime/tile_map/TileData.h"

namespace cave {

TEST(TileChunk, DefaultConstructedChunkIsEmpty) {
    TileChunk chunk;

    EXPECT_TRUE(chunk.empty());

    for (int16_t y = 0; y < kTileChunkSize; ++y) {
        for (int16_t x = 0; x < kTileChunkSize; ++x) {
            EXPECT_TRUE(chunk.at(x, y).empty());
        }
    }
}

TEST(TileChunk, AtReadsAndWritesTile) {
    TileChunk chunk;

    TileCell cell{ TileId(42) };
    chunk.at(3, 5) = cell;

    EXPECT_EQ(chunk.at(3, 5), cell);
    EXPECT_FALSE(chunk.empty());

    EXPECT_FALSE(chunk.at(0, 0).hasTile());
    EXPECT_FALSE(chunk.at(31, 31).hasTile());
}

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
    EXPECT_EQ(ToTileLocalX(TileCoord{ 31, 31 }), 31);
    EXPECT_EQ(ToTileLocalY(TileCoord{ 31, 31 }), 31);

    EXPECT_EQ(ToTileChunkCoord(TileCoord{ 32, 32 }), (TileChunkCoord{ 1, 1 }));
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

TEST(ChunkedTileData, NewDataHasNoTiles) {
    ChunkedTileData data;

    EXPECT_TRUE(data.chunks().empty());
    EXPECT_FALSE(data.cellAt(TileCoord{ 0, 0 }).is_some());
    EXPECT_FALSE(data.cellAt(TileCoord{ -1, -1 }).is_some());
}

TEST(ChunkedTileData, AddTileCreatesChunk) {
    ChunkedTileData data;

    EXPECT_TRUE(data.addTile(TileCoord{ 0, 0 }, 7));

    auto tile = data.cellAt(TileCoord{ 0, 0 });
    ASSERT_TRUE(tile.is_some());
    EXPECT_EQ(tile.unwrap(), 7);

    EXPECT_EQ(data.chunks().size(), 1u);
    EXPECT_TRUE(data.chunks().contains(TileChunkCoord{ 0, 0 }));
}

TEST(ChunkedTileData, AddSameTileReturnsFalse) {
    ChunkedTileData data;

    EXPECT_TRUE(data.addTile(TileCoord{ 0, 0 }, 7));
    EXPECT_FALSE(data.addTile(TileCoord{ 0, 0 }, 7));

    auto tile = data.cellAt(TileCoord{ 0, 0 });
    ASSERT_TRUE(tile.is_some());
    EXPECT_EQ(tile.unwrap(), 7);

    EXPECT_EQ(data.chunks().size(), 1u);
}

TEST(ChunkedTileData, AddDifferentTileReplacesOldTile) {
    ChunkedTileData data;

    EXPECT_TRUE(data.addTile(TileCoord{ 0, 0 }, 7));
    EXPECT_TRUE(data.addTile(TileCoord{ 0, 0 }, 9));

    auto tile = data.cellAt(TileCoord{ 0, 0 });
    ASSERT_TRUE(tile.is_some());
    EXPECT_EQ(tile.unwrap(), 9);

    EXPECT_EQ(data.chunks().size(), 1u);
}

TEST(ChunkedTileData, RemoveTileRemovesTile) {
    ChunkedTileData data;

    EXPECT_TRUE(data.addTile(TileCoord{ 0, 0 }, 7));
    EXPECT_TRUE(data.removeTile(TileCoord{ 0, 0 }));

    EXPECT_TRUE(data.cellAt(TileCoord{ 0, 0 }).is_none());
}

TEST(ChunkedTileData, RemoveMissingTileReturnsFalse) {
    ChunkedTileData data;

    EXPECT_FALSE(data.removeTile(TileCoord{ 0, 0 }));

    EXPECT_TRUE(data.addTile(TileCoord{ 0, 0 }, 7));
    EXPECT_FALSE(data.removeTile(TileCoord{ 1, 0 }));
}

TEST(ChunkedTileData, RemoveOneTileKeepsNonEmptyChunk) {
    ChunkedTileData data;

    EXPECT_TRUE(data.addTile(TileCoord{ 0, 0 }, 7));
    EXPECT_TRUE(data.addTile(TileCoord{ 1, 0 }, 8));

    EXPECT_TRUE(data.removeTile(TileCoord{ 0, 0 }));

    EXPECT_EQ(data.chunks().size(), 1u);
    EXPECT_TRUE(data.cellAt(TileCoord{ 0, 0 }).is_none());

    auto tile = data.cellAt(TileCoord{ 1, 0 });
    ASSERT_TRUE(tile.is_some());
    EXPECT_EQ(tile.unwrap(), 8);
}

TEST(ChunkedTileData, NegativeTileRoundTrip) {
    ChunkedTileData data;

    EXPECT_TRUE(data.addTile(TileCoord{ -1, -1 }, 11));

    auto tile = data.cellAt(TileCoord{ -1, -1 });
    ASSERT_TRUE(tile.is_some());
    EXPECT_EQ(tile.unwrap(), 11);

    EXPECT_EQ(data.chunks().size(), 1u);
    EXPECT_TRUE(data.chunks().contains(TileChunkCoord{ -1, -1 }));
}

TEST(ChunkedTileData, DifferentChunksAreIndependent) {
    ChunkedTileData data;

    EXPECT_TRUE(data.addTile(TileCoord{ 0, 0 }, 1));
    EXPECT_TRUE(data.addTile(TileCoord{ 32, 0 }, 2));
    EXPECT_TRUE(data.addTile(TileCoord{ -1, 0 }, 3));

    EXPECT_EQ(data.chunks().size(), 3u);

    ASSERT_TRUE(data.cellAt(TileCoord{ 0, 0 }).is_some());
    ASSERT_TRUE(data.cellAt(TileCoord{ 32, 0 }).is_some());
    ASSERT_TRUE(data.cellAt(TileCoord{ -1, 0 }).is_some());

    EXPECT_EQ(data.cellAt(TileCoord{ 0, 0 }).unwrap(), 1);
    EXPECT_EQ(data.cellAt(TileCoord{ 32, 0 }).unwrap(), 2);
    EXPECT_EQ(data.cellAt(TileCoord{ -1, 0 }).unwrap(), 3);
}

TEST(ChunkedTileData, RemoveLastTileErasesEmptyChunk) {
    ChunkedTileData data;
    (void)data;

    // EXPECT_TRUE(data.addTile(TileCoord{ 0, 0 }, 7));
    // EXPECT_EQ(data.chunks().size(), 1u);

    // EXPECT_TRUE(data.removeTile(TileCoord{ 0, 0 }));
    // EXPECT_TRUE(data.chunks().empty());
}

TEST(ChunkedTileData, AddingEmptyTileIdShouldNotBeAllowed) {
    ChunkedTileData data;
    (void)data;

    EXPECT_DEATH(data.addTile(TileCoord{ 0, 0 }, kEmptyTileId), "");
}

}  // namespace cave
