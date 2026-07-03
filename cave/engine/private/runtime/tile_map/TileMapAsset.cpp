#include "cave/runtime/tile_map/TileMapAsset.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "engine/private/core/io/file_access.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/serialization/yaml_include.h"

namespace cave {

namespace {

int16_t DivFloor(int16_t a, int16_t b = kTileChunkSize) {
    return (a >= 0) ? (a / b) : ((a - b + 1) / b);
}

}  // namespace

void TileMapAsset::tileSetGuid(const Guid& guid, bool force_update) {
    const bool should_update = force_update || tile_set_id_ != guid;
    if (should_update) {
        if (auto handle = AssetRegistry::singleton().findByGuid<TileSetAsset>(guid); handle.is_some()) {
            tile_set_id_ = guid;
            tile_set_handle_ = std::move(handle.unwrap());
        } else {
            tile_set_id_ = Guid::null();
            tile_set_handle_.Invalidate();
        }

        incRevision();
    }
}

std::vector<Guid> TileMapAsset::dependencies() const {
    return { tile_set_id_ };
}

ISerializer& WriteObject(ISerializer& s, const ChunkedTileData& tile_data) {
    s.BeginArray(false);

    auto chunk_empty = [](const TileChunk& p_chunk) {
        for (int16_t y = 0; y < kTileChunkSize; ++y) {
            for (int16_t x = 0; x < kTileChunkSize; ++x) {
                if (p_chunk.at(x, y) != kEmptyTileId) {
                    return false;
                }
            }
        }
        return true;
    };

    for (const auto& [index, chunk] : tile_data.chunks()) {
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

        for (int16_t y = 0; y < kTileChunkSize; ++y) {
            for (int16_t x = 0; x < kTileChunkSize; ++x) {
                s.Write(chunk->at(x, y));
            }
        }

        s.EndArray()
            .EndMap();
    }

    return s.EndArray();
}

bool ReadObject(IDeserializer& d, ChunkedTileData& tile_data) {
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
                tile_data.chunks()[TileChunkCoord(x, y)] = std::move(chunk);

                constexpr int TILE_COUNT = kTileChunkSize * kTileChunkSize;
                DEV_ASSERT(d.ArraySize().unwrap_or(0) == TILE_COUNT);
                for (int tile_idx = 0; tile_idx < TILE_COUNT; ++tile_idx) {
                    DEV_ASSERT(d.TryEnterIndex(tile_idx));
                    d.Read(tiles[tile_idx]);
                    d.LeaveIndex();
                }
                d.LeaveKey();
            }
        }

        d.LeaveIndex();
    }

    return true;
}

Result<void> TileMapAsset::saveToDisk(const AssetMetaData& meta) const {
    auto res = meta.saveToDisk(this);
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
    return SaveYaml(meta.import_path, yaml);
}

Result<void> TileMapAsset::loadFromDisk(const AssetMetaData& meta) {
    YAML::Node root;

    if (auto res = LoadYaml(meta.import_path, root); !res) {
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

    tileSetGuid(tile_set_id_, true);
    return Result<void>();
}

}  // namespace cave
