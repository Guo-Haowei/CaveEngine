#include "tile_map_asset.h"

#include "engine/assets/assets.h"
#include "engine/assets/sprite_asset.h"
#include "engine/core/io/file_access.h"
#include "engine/runtime/asset_registry.h"
#include "engine/serialization/serialization.h"

namespace my {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TileIndex, x, y);

TileData TileMapAsset::GetTile(TileIndex p_index) {
    auto it = m_tiles.find(p_index);
    if (it == m_tiles.end()) {
        return TILE_DATA_EMPTY;
    }

    return it->second;
}

void TileMapAsset::SetTiles(std::unordered_map<TileIndex, TileData>&& p_tiles) {
    m_tiles = std::move(p_tiles);

    IncRevision();
}

void TileMapAsset::SetSpriteGuid(const Guid& p_guid) {
    if (m_sprite_guid != p_guid) {
        if (auto handle = AssetRegistry::GetSingleton().FindByGuid<SpriteAsset>(p_guid); handle) {
            m_sprite_guid = p_guid;
            m_sprite_handle = std::move(*handle);
        } else {
            m_sprite_guid = Guid::Null();
            m_sprite_handle.Invalidate();
        }

        IncRevision();
    }
}

// @TODO: this should be in tile map edit module
TileResult TileMapAsset::SetTile(TileIndex p_index, TileData p_new_tile, TileData& p_out_old) {
    p_out_old = TILE_DATA_EMPTY;

    auto it = m_tiles.find(p_index);

    // if there's a tile
    if (it != m_tiles.end()) {
        p_out_old = it->second;  // save the old tile

        if (p_new_tile == TILE_DATA_EMPTY) {
            // do nothing, the tile was already empty
            m_tiles.erase(it);

            return TileResult::Removed;
        }

        // if the tiles are the same, do nothing
        if (it->second == p_new_tile) {
            return TileResult::Noop;
        }

        it->second = p_new_tile;
        return TileResult::Replaced;
    }

    // if there isn't a tile
    if (p_new_tile == TILE_DATA_EMPTY) {
        // can't replace an empty tile with an empty tile
        return TileResult::Noop;
    }

    m_tiles.insert({ p_index, p_new_tile });
    return TileResult::Added;
}

std::vector<Guid> TileMapAsset::GetDependencies() const {
    return { m_sprite_guid };
}

auto TileMapAsset::SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> {
    // meta
    auto res = p_meta.SaveToDisk(this);
    if (!res) {
        return HBN_ERROR(res.error());
    }

    json j;
    j["version"] = VERSION;
    j["name"] = m_name;
    j["sprite_guid"] = m_sprite_guid;
    j["tiles"] = m_tiles;

    return Serialize(p_meta.path, j);
}

void TileMapAsset::LoadFromDiskVersion1(const json& j) {
    auto name = j["name"].get<std::string>();
    SetName(std::move(name));
    SetSpriteGuid(j["sprite_guid"].get<Guid>());

    auto tiles = j["tiles"].get<std::unordered_map<TileIndex, TileData>>();

    SetTiles(std::move(tiles));
}

void TileMapAsset::LoadFromDiskVersion0(const json& j) {
    auto layers = j["layers"];
    if (layers.is_array() && !layers.empty()) {
        const json& first_layer = layers.front();
        LoadFromDiskVersion1(first_layer);
    }
}

auto TileMapAsset::LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> {
    json j;

    if (auto res = Deserialize(p_meta.path, j); !res) {
        return HBN_ERROR(res.error());
    }

    const int version = j["version"];

    try {
        switch (version) {
            case 0:
                LoadFromDiskVersion0(j);
                break;
            default:
                LoadFromDiskVersion1(j);
                break;
        }
    } catch (const json::exception& e) {
        return HBN_ERROR(ErrorCode::ERR_PARSE_ERROR, "{}", e.what());
    }

    return Result<void>();
}

}  // namespace my
