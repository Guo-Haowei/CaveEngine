#include "cave/runtime/tile_map/TileMapAsset.h"

#include "engine/private/core/io/file_access.h"
#include "engine/private/runtime/assets/TileSetAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/serialization/yaml_include.h"

namespace cave {

static int16_t DivFloor(int16_t a, int16_t b = kTileChunkSize) {
    return (a >= 0) ? (a / b) : ((a - b + 1) / b);
}

TileCoord TileMapAsset::convertIndex(TileCoord p_index) const {
    return TileCoord{ DivFloor(p_index.x), DivFloor(p_index.y) };
}

Option<TileId> TileMapAsset::tileAt(TileCoord p_index) const {
    TileCoord index = convertIndex(p_index);

    auto it = m_tiles.chunks.find(index);
    if (it == m_tiles.chunks.end()) {
        return None();
    }

    const int16_t x = p_index.x - index.x * kTileChunkSize;
    const int16_t y = p_index.y - index.y * kTileChunkSize;
    DEV_ASSERT_INDEX(x, kTileChunkSize);
    DEV_ASSERT_INDEX(y, kTileChunkSize);

    return Some(it->second->tiles[y][x]);
}

bool TileMapAsset::addTile(TileCoord p_index, TileId p_id) {
    TileCoord index = convertIndex(p_index);

    auto& chunk = m_tiles.chunks[index];
    if (chunk == nullptr) {
        chunk = std::make_unique<TileChunk>();
        std::memset(chunk.get(), 0xFFFFFFFF, sizeof(TileChunk));
    }

    const int16_t x = p_index.x - index.x * kTileChunkSize;
    const int16_t y = p_index.y - index.y * kTileChunkSize;

    TileId& tile_id = chunk->tiles[y][x];
    if (tile_id == p_id) {
        return false;
    }

    tile_id = p_id;
    return true;
}

bool TileMapAsset::removeTile(TileCoord p_index) {
    TileCoord index = convertIndex(p_index);

    auto it = m_tiles.chunks.find(index);
    if (it == m_tiles.chunks.end()) {
        return false;
    }

    const int16_t x = p_index.x - index.x * kTileChunkSize;
    const int16_t y = p_index.y - index.y * kTileChunkSize;

    TileId& tile = it->second->tiles[y][x];
    if (tile == kEmptyTileId) {
        return false;
    }

    tile = kEmptyTileId;
    return true;
}

void TileMapAsset::SetTileSetGuid(const Guid& p_guid, bool p_force) {
    const bool should_update = p_force || m_tile_set_id != p_guid;
    if (should_update) {
        if (auto handle = AssetRegistry::singleton().FindByGuid<TileSetAsset>(p_guid); handle.is_some()) {
            m_tile_set_id = p_guid;
            m_tile_set_handle = std::move(handle.unwrap());
        } else {
            m_tile_set_id = Guid::Null();
            m_tile_set_handle.Invalidate();
        }

        incRevision();
    }
}

std::vector<Guid> TileMapAsset::GetDependencies() const {
    return { m_tile_set_id };
}

ISerializer& WriteObject(ISerializer& s, const TileData& p_tile_data) {
    s.BeginArray(false);

    auto chunk_empty = [](const TileChunk& p_chunk) {
        for (int y = 0; y < kTileChunkSize; ++y) {
            for (int x = 0; x < kTileChunkSize; ++x) {
                if (p_chunk.tiles[y][x] != kEmptyTileId) {
                    return false;
                }
            }
        }
        return true;
    };

    for (const auto& [index, chunk] : p_tile_data.chunks) {
        if (chunk_empty(*chunk.get())) {
            continue;
        }

        s.BeginMap(false)
            .Key("x")
            .Write(index.x)
            .Key("y")
            .Write(index.y)
            .Key("tiles")
            .BeginArray(true);

        for (int y = 0; y < kTileChunkSize; ++y) {
            for (int x = 0; x < kTileChunkSize; ++x) {
                s.Write(chunk->tiles[y][x]);
            }
        }

        s.EndArray()
            .EndMap();
    }

    return s.EndArray();
}

bool ReadObject(IDeserializer& d, TileData& p_tile_data) {
    const int chunk_size = d.ArraySize().unwrap_or(-1);
    if (chunk_size < 0) {
        return false;
    }

    for (int chunk_idx = 0; chunk_idx < chunk_size; ++chunk_idx) {
        DEV_ASSERT(d.TryEnterIndex(chunk_idx));
        int16_t x = INT16_MAX;
        int16_t y = INT16_MAX;
        if (DEV_VERIFY(d.TryEnterKey("x"))) {
            d.Read(x);
            d.LeaveKey();
        }
        if (DEV_VERIFY(d.TryEnterKey("y"))) {
            d.Read(y);
            d.LeaveKey();
        }

        if (x != INT16_MAX && y != INT16_MAX) {
            if (d.TryEnterKey("tiles")) {
                auto chunk = std::make_unique<TileChunk>();
                auto& tiles = chunk->tiles;
                p_tile_data.chunks[TileCoord(x, y)] = std::move(chunk);

                constexpr int TILE_COUNT = kTileChunkSize * kTileChunkSize;
                DEV_ASSERT(d.ArraySize().unwrap_or(0) == TILE_COUNT);
                for (int tile_idx = 0; tile_idx < TILE_COUNT; ++tile_idx) {
                    DEV_ASSERT(d.TryEnterIndex(tile_idx));
                    d.Read(tiles[tile_idx / kTileChunkSize][tile_idx % kTileChunkSize]);
                    d.LeaveIndex();
                }
                d.LeaveKey();
            }
        }

        d.LeaveIndex();
    }

    return true;
}

Result<void> TileMapAsset::SaveToDisk(const AssetMetaData& p_meta) const {
    auto res = p_meta.SaveToDisk(this);
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    YamlSerializer yaml;
    yaml.BeginMap(false)
        .Key("version")
        .Write(VERSION)
        .Key("content")
        .Write(*this)
        .EndMap();
    return SaveYaml(p_meta.import_path, yaml);
}

Result<void> TileMapAsset::LoadFromDisk(const AssetMetaData& p_meta) {
    YAML::Node root;

    if (auto res = LoadYaml(p_meta.import_path, root); !res) {
        return CAVE_ERROR(res.error());
    }

    YamlDeserializer d;
    d.Initialize(root);

    const int version = d.GetVersion();

    if (d.TryEnterKey("content")) {
        switch (version) {
            case 1:
                [[fallthrough]];
            default:
                d.Read(*this);
                break;
        }

        d.LeaveKey();
    }

    SetTileSetGuid(m_tile_set_id, true);
    return Result<void>();
}

}  // namespace cave
