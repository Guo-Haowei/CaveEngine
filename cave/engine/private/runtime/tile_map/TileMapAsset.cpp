#include "cave/runtime/tile_map/TileMapAsset.h"
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "engine/private/core/io/file_access.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/serialization/YamlInclude.h"

namespace cave {

namespace {

int16_t DivFloor(int16_t a, int16_t b = kTileChunkSize) {
    return (a >= 0) ? (a / b) : ((a - b + 1) / b);
}

}  // namespace

void TileMapAsset::tileSetGuid(const Guid& guid, bool force_update) {
    const bool should_update = force_update || m_tile_set_id != guid;
    if (should_update) {
        if (auto handle = AssetRegistry::singleton().findByGuid<TileSetAsset>(guid)) {
            m_tile_set_id = guid;
            m_tile_set_handle = std::move(handle.unwrap());
        } else {
            m_tile_set_id = Guid::null();
            m_tile_set_handle.invalidate();
        }

        incRevision();
    }
}

Vector<Guid> TileMapAsset::dependencies() const {
    return { m_tile_set_id };
}

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
                s.write(chunk->at(x, y));
            }
        }

        s.endArray()
            .endMap();
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
                            d.read(chunk->at(local_x, local_y));
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

Result<void> TileMapAsset::saveToDisk(const AssetMetaData& meta) const {
    auto res = meta.saveToDisk(this);
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    YamlSerializer yaml;
    yaml.beginMap(false)
        .beginKey("version")
        .write(kVersion)
        .beginKey("content")
        .write(*this)
        .endMap();
    return SaveYaml(meta.import_path, yaml);
}

Result<void> TileMapAsset::loadFromDisk(const AssetMetaData& meta) {
    YAML::Node root;

    if (auto res = LoadYaml(meta.import_path, root); !res) {
        return CAVE_ERROR(res.error());
    }

    YamlDeserializer d;
    d.initialize(root);

    const int version = d.version();

    if (d.tryEnterKey("content")) {
        switch (version) {
            case 1:
                [[fallthrough]];
            default:
                d.read(*this);
                break;
        }

        d.leaveKey();
    }

    tileSetGuid(m_tile_set_id, true);
    return Result<void>();
}

}  // namespace cave
