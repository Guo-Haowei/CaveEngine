#include "tile_map_asset.h"

// #include "engine/assets/blob_asset.h"
#include "engine/assets/tile_set_asset.h"
#include "engine/core/io/file_access.h"
#include "engine/runtime/asset_registry.h"
#include "engine/serialization/yaml_include.h"

namespace cave {

static int16_t DivFloor(int16_t a, int16_t b = TILE_CHUNK_SIZE) {
    return (a >= 0) ? (a / b) : ((a - b + 1) / b);
}

TileIndex TileMapAsset::ConvertIndex(TileIndex p_index) const {
    return TileIndex{ DivFloor(p_index.x), DivFloor(p_index.y) };
}

Option<TileId> TileMapAsset::GetTile(TileIndex p_index) const {
    TileIndex index = ConvertIndex(p_index);

    auto it = m_tiles.chunks.find(index);
    if (it == m_tiles.chunks.end()) {
        return None();
    }

    const int16_t x = p_index.x - index.x * TILE_CHUNK_SIZE;
    const int16_t y = p_index.y - index.y * TILE_CHUNK_SIZE;
    DEV_ASSERT_INDEX(x, TILE_CHUNK_SIZE);
    DEV_ASSERT_INDEX(y, TILE_CHUNK_SIZE);

    return Some(it->second->tiles[y][x]);
}

bool TileMapAsset::AddTile(TileIndex p_index, TileId p_id) {
    TileIndex index = ConvertIndex(p_index);

    auto it = m_tiles.chunks.find(index);

    auto& chunk = m_tiles.chunks[index];
    if (chunk == nullptr) {
        chunk = std::make_unique<TileChunk>();
        std::memset(chunk.get(), 0xFFFFFFFF, sizeof(TileChunk));
    }

    const int16_t x = p_index.x - index.x * TILE_CHUNK_SIZE;
    const int16_t y = p_index.y - index.y * TILE_CHUNK_SIZE;

    TileId& tile_id = chunk->tiles[y][x];
    if (tile_id == p_id) {
        return false;
    }

    tile_id = p_id;
    return true;
}

bool TileMapAsset::RemoveTile(TileIndex p_index) {
    TileIndex index = ConvertIndex(p_index);

    auto it = m_tiles.chunks.find(index);
    if (it == m_tiles.chunks.end()) {
        return false;
    }

    const int16_t x = p_index.x - index.x * TILE_CHUNK_SIZE;
    const int16_t y = p_index.y - index.y * TILE_CHUNK_SIZE;

    TileId& tile = it->second->tiles[y][x];
    if (tile == TILE_ID_EMPTY) {
        return false;
    }

    tile = TILE_ID_EMPTY;
    return true;
}

void TileMapAsset::SetTileSetGuid(const Guid& p_guid, bool p_force) {
    const bool should_update = p_force || m_tile_set_id != p_guid;
    if (should_update) {
        if (auto handle = AssetRegistry::GetSingleton().FindByGuid<TileSetAsset>(p_guid); handle.is_some()) {
            m_tile_set_id = p_guid;
            m_tile_set_handle = std::move(handle.unwrap());
        } else {
            m_tile_set_id = Guid::Null();
            m_tile_set_handle.Invalidate();
        }

        IncRevision();
    }
}

std::vector<Guid> TileMapAsset::GetDependencies() const {
    return { m_tile_set_id };
}

ISerializer& WriteObject(ISerializer& s, const TileData& p_tile_data) {
    s.BeginArray(false);

    auto chunk_empty = [](const TileChunk& p_chunk) {
        for (int y = 0; y < TILE_CHUNK_SIZE; ++y) {
            for (int x = 0; x < TILE_CHUNK_SIZE; ++x) {
                if (p_chunk.tiles[y][x] != TILE_ID_EMPTY) {
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

        for (int y = 0; y < TILE_CHUNK_SIZE; ++y) {
            for (int x = 0; x < TILE_CHUNK_SIZE; ++x) {
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
                p_tile_data.chunks[TileIndex(x, y)] = std::move(chunk);

                constexpr int TILE_COUNT = TILE_CHUNK_SIZE * TILE_CHUNK_SIZE;
                DEV_ASSERT(d.ArraySize().unwrap_or(0) == TILE_COUNT);
                for (int tile_idx = 0; tile_idx < TILE_COUNT; ++tile_idx) {
                    DEV_ASSERT(d.TryEnterIndex(tile_idx));
                    d.Read(tiles[tile_idx / TILE_CHUNK_SIZE][tile_idx % TILE_CHUNK_SIZE]);
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
    return SaveYaml(p_meta.path, yaml);
}

Result<void> TileMapAsset::LoadFromDisk(const AssetMetaData& p_meta) {
    YAML::Node root;

    if (auto res = LoadYaml(p_meta.path, root); !res) {
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
