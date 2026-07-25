#include "cave/runtime/tile_map/TileData.h"

#include "engine/private/runtime/serialization/Deserializer.h"
#include "engine/private/runtime/serialization/Serializer.h"

namespace cave {

namespace {

// kChunkedTileDataVersion history
// version 1: initial version
// version 2: add terrain id
constexpr int kChunkedTileDataVersion = 2;

int16_t FloorDiv(int16_t a, int16_t b) {
    return (a >= 0) ? (a / b) : ((a - b + 1) / b);
}

int16_t PositiveMod(int16_t value, int16_t divisor) {
    int16_t r = value % divisor;
    if (r < 0) {
        r += divisor;
    }
    return r;
}

}  // namespace

TileChunkCoord ToTileChunkCoord(TileCoord coord) {
    return TileChunkCoord{
        FloorDiv(coord.x, kTileChunkSize),
        FloorDiv(coord.y, kTileChunkSize),
    };
}

int16_t ToTileLocalX(TileCoord coord) {
    return PositiveMod(coord.x, kTileChunkSize);
}

int16_t ToTileLocalY(TileCoord coord) {
    return PositiveMod(coord.y, kTileChunkSize);
}

TileCoord ToTileCoord(TileChunkCoord chunk_coord, int16_t local_x, int16_t local_y) {
    DEV_ASSERT(local_x >= 0 && local_x < kTileChunkSize);
    DEV_ASSERT(local_y >= 0 && local_y < kTileChunkSize);

    return TileCoord{
        static_cast<int16_t>(chunk_coord.x * kTileChunkSize + local_x),
        static_cast<int16_t>(chunk_coord.y * kTileChunkSize + local_y),
    };
}

ChunkedTileData::ChunkedTileData(const ChunkedTileData& other) {
    for (const auto& [coord, chunk] : other.m_chunks) {
        if (chunk) {
            m_chunks.emplace(
                coord,
                std::make_unique<TileChunk>(*chunk));
        } else {
            m_chunks.emplace(coord, nullptr);
        }
    }
}

ChunkedTileData& ChunkedTileData::operator=(const ChunkedTileData& other) {
    if (this == &other) {
        return *this;
    }

    ChunkedTileData copy(other);
    std::swap(copy.m_chunks, m_chunks);
    return *this;
}

bool TileChunk::empty() const {
    for (TileCell cell : m_local_tiles) {
        if (!cell.empty()) {
            return false;
        }
    }
    return true;
}

Option<TileCell> ChunkedTileData::cellAt(TileCoord coord) const {
    TileChunkCoord chunk_coord = ToTileChunkCoord(coord);

    auto it = m_chunks.find(chunk_coord);
    if (it == m_chunks.end()) {
        return None();
    }

    const int16_t x = coord.x - chunk_coord.x * kTileChunkSize;
    const int16_t y = coord.y - chunk_coord.y * kTileChunkSize;
    DEV_ASSERT_INDEX(x, kTileChunkSize);
    DEV_ASSERT_INDEX(y, kTileChunkSize);

    TileCell tile = it->second->at(x, y);
    if (tile.empty()) {
        return None();
    }
    return Some(tile);
}

bool ChunkedTileData::setCell(TileCoord coord, TileCell cell) {
    DEV_ASSERT(!cell.empty());

    TileChunkCoord chunk_coord = ToTileChunkCoord(coord);

    auto& chunk = m_chunks[chunk_coord];
    if (chunk == nullptr) {
        chunk = MakeOwner<TileChunk>();
    }

    TileCell& current = chunk->at(ToTileLocalX(coord), ToTileLocalY(coord));
    if (current == cell) {
        return false;
    }

    current = cell;
    return true;
}

bool ChunkedTileData::removeCell(TileCoord coord) {
    TileChunkCoord chunk_coord = ToTileChunkCoord(coord);

    auto it = m_chunks.find(chunk_coord);
    if (it == m_chunks.end()) {
        return false;
    }

    TileCell& cell = it->second->at(ToTileLocalX(coord), ToTileLocalY(coord));
    if (cell.empty()) {
        return false;
    }

    cell = TileCell{};
    if (it->second->empty()) m_chunks.erase(it);
    return true;
}

bool ChunkedTileData::addChunk(TileChunkCoord coord, Owner<TileChunk>&& chunk) {
    auto [it, inserted] = m_chunks.insert(std::make_pair(coord, std::move(chunk)));

    return inserted;
}

static_assert(Serializable<ChunkedTileData>);

ISerializer& WriteObject(ISerializer& s, const ChunkedTileData& tile_data) {
    s.beginArray(false);

    for (const auto& [index, chunk] : tile_data.chunks()) {
        if (!chunk || chunk->empty()) continue;

        s.beginMap(false)
            .beginKey("version")
            .write(kChunkedTileDataVersion)
            .beginKey("x")
            .write(index.x)
            .beginKey("y")
            .write(index.y)
            .beginKey("tiles")
            .beginArray(true);

        for (int16_t y = 0; y < kTileChunkSize; ++y) {
            for (int16_t x = 0; x < kTileChunkSize; ++x) {
                const TileCell& cell = chunk->at(x, y);
                s.beginArray(true).write(cell.tile_id.value).write(cell.terrain_id.value).endArray();
            }
        }

        s.endArray().endMap();
    }

    return s.endArray();
}

bool ReadObject(IDeserializer& d, ChunkedTileData& tile_data) {
    const int chunk_count = d.arraySize().unwrap_or(-1);
    if (chunk_count < 0) return false;

    constexpr int16_t kInvalidChunkCoord = std::numeric_limits<int16_t>::max();

    for (int chunk_idx = 0; chunk_idx < chunk_count; ++chunk_idx) {
        if (!DEV_VERIFY(d.tryEnterIndex(chunk_idx))) continue;

        int version = 1;
        int16_t chunk_x = kInvalidChunkCoord;
        int16_t chunk_y = kInvalidChunkCoord;

        if (d.tryEnterKey("version")) {
            d.read(version);
            d.leaveKey();
        }

        if (DEV_VERIFY(d.tryEnterKey("x"))) {
            d.read(chunk_x);
            d.leaveKey();
        }

        if (DEV_VERIFY(d.tryEnterKey("y"))) {
            d.read(chunk_y);
            d.leaveKey();
        }

        if (chunk_x == kInvalidChunkCoord || chunk_y == kInvalidChunkCoord) {
            d.leaveIndex();
            continue;
        }

        if (!d.tryEnterKey("tiles")) {
            d.leaveIndex();
            continue;
        }

        auto chunk = MakeOwner<TileChunk>();
        DEV_ASSERT(d.arraySize().unwrap_or(0) == kTileChunkArea);

        for (int16_t local_y = 0; local_y < kTileChunkSize; ++local_y) {
            for (int16_t local_x = 0; local_x < kTileChunkSize; ++local_x) {
                const int tile_idx = local_y * kTileChunkSize + local_x;
                if (!DEV_VERIFY(d.tryEnterIndex(tile_idx))) continue;

                TileCell& cell = chunk->at(local_x, local_y);

                if (version <= 1) {
                    d.read(cell.tile_id.value);
                } else {
                    const int cell_size = d.arraySize().unwrap_or(-1);

                    if (DEV_VERIFY(cell_size == 2)) {
                        if (DEV_VERIFY(d.tryEnterIndex(0))) {
                            d.read(cell.tile_id.value);
                            d.leaveIndex();
                        }

                        if (DEV_VERIFY(d.tryEnterIndex(1))) {
                            d.read(cell.terrain_id.value);
                            d.leaveIndex();
                        }
                    }
                }

                d.leaveIndex();
            }
        }

        if (!chunk->empty()) {
            const bool inserted = tile_data.addChunk(TileChunkCoord{ chunk_x, chunk_y }, std::move(chunk));
            DEV_ASSERT(inserted);
        }

        d.leaveKey();
        d.leaveIndex();
    }

    return true;
}

}  // namespace cave
