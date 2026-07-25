#include "cave/runtime/tile_map/TileData.h"

#include "engine/private/runtime/serialization/Deserializer.h"
#include "engine/private/runtime/serialization/Serializer.h"

namespace cave {

namespace {

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
        if (chunk->empty()) {
            continue;
        }

        s.beginMap(false)
            .beginKey("x")
            .write(index.x)
            .beginKey("y")
            .write(index.y)
            .beginKey("tiles")
            .beginArray(true);

        for (int16_t y = 0; y < kTileChunkSize; ++y) {
            for (int16_t x = 0; x < kTileChunkSize; ++x) {
                s.write(chunk->at(x, y).tile_id.value);
            }
        }

        s.endArray().endMap();
    }

    return s.endArray();
}

bool ReadObject(IDeserializer& d, ChunkedTileData& tile_data) {
    const int chunk_size = d.arraySize().unwrap_or(-1);
    if (chunk_size < 0) {
        return false;
    }

    for (int chunk_idx = 0; chunk_idx < chunk_size; ++chunk_idx) {
        DEV_ASSERT(d.tryEnterIndex(chunk_idx));
        constexpr int16_t kMaxIndex = std::numeric_limits<int16_t>::max();
        int16_t x = kMaxIndex;
        int16_t y = kMaxIndex;
        if (DEV_VERIFY(d.tryEnterKey("x"))) {
            d.read(x);
            d.leaveKey();
        }
        if (DEV_VERIFY(d.tryEnterKey("y"))) {
            d.read(y);
            d.leaveKey();
        }

        if (x != kMaxIndex && y != kMaxIndex) {
            if (d.tryEnterKey("tiles")) {
                auto chunk = MakeOwner<TileChunk>();

                DEV_ASSERT(d.arraySize().unwrap_or(0) == kTileChunkArea);
                for (int16_t local_y = 0; local_y < kTileChunkSize; ++local_y) {
                    for (int16_t local_x = 0; local_x < kTileChunkSize; ++local_x) {
                        const int16_t tile_idx = local_y * kTileChunkSize + local_x;
                        if (DEV_VERIFY(d.tryEnterIndex(tile_idx))) {
                            TileCell& cell = chunk->at(local_x, local_y);
                            d.read(cell.tile_id.value);
                            d.leaveIndex();
                        }
                    }
                }

                if (!chunk->empty()) {
                    [[maybe_unused]]
                    const bool inserted = tile_data.addChunk(TileChunkCoord(x, y), std::move(chunk));
                    DEV_ASSERT(inserted);
                }
                d.leaveKey();
            }
        }

        d.leaveIndex();
    }

    return true;
}

}  // namespace cave
