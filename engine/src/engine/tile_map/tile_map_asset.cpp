#include "tile_map_asset.h"

#include "engine/assets/assets.h"
#include "engine/assets/sprite_asset.h"
#include "engine/core/io/file_access.h"
#include "engine/runtime/asset_registry.h"
#include "engine/serialization/yaml_include.h"

namespace cave {

Option<TileId> TileMapAsset::GetTile(TileIndex p_index) const {
    auto it = m_tiles.tiles.find(p_index);
    if (it == m_tiles.tiles.end()) {
        return None();
    }

    return Some(it->second);
}

bool TileMapAsset::AddTile(TileIndex p_index, TileId p_data) {
    auto [it, inserted] = m_tiles.tiles.try_emplace(p_index, p_data);
    if (inserted) {
        return true;
    }

    // same tile, nothing changed
    if (it->second == p_data) {
        return false;
    }

    it->second = p_data;
    return true;
}

bool TileMapAsset::RemoveTile(TileIndex p_index) {
    auto it = m_tiles.tiles.find(p_index);
    if (it == m_tiles.tiles.end()) {
        return false;
    }

    m_tiles.tiles.erase(it);
    return true;
}

void TileMapAsset::SetTiles(TileChunk&& p_tiles) {
    m_tiles.tiles = std::move(p_tiles);

    IncRevision();
}

void TileMapAsset::SetSpriteGuid(const Guid& p_guid, bool p_force) {
    const bool should_update = p_force || m_sprite_guid != p_guid;
    if (should_update) {
        if (auto handle = AssetRegistry::GetSingleton().FindByGuid<SpriteAsset>(p_guid); handle.is_some()) {
            m_sprite_guid = p_guid;
            m_sprite_handle = std::move(handle.unwrap());
        } else {
            m_sprite_guid = Guid::Null();
            m_sprite_handle.Invalidate();
        }

        IncRevision();
    }
}

ISerializer& WriteObject(ISerializer& p_serializer, const TileData& p_tile_data) {
    p_serializer.BeginArray(false);
    for (const auto& [index, id] : p_tile_data.tiles) {
        p_serializer
            .BeginArray(true)
            .Write(index.x)
            .Write(index.y)
            .Write(id)
            .EndArray();
    }
    return p_serializer.EndArray();
}

bool ReadObject(IDeserializer& p_deserializer, TileData& p_tile_data) {
    auto& deserializer = static_cast<YamlDeserializer&>(p_deserializer);

    const auto& node = deserializer.Current();
    ERR_FAIL_COND_V_MSG(!node || !node.IsSequence(), false, "invalid tile map");

    for (size_t idx = 0; idx < node.size(); ++idx) {
        const auto& tile = node[idx];
        if (tile && tile.IsSequence() && tile.size() == 3) {
            auto x = tile[0].as<int16_t>();
            auto y = tile[1].as<int16_t>();
            auto id = tile[2].as<uint32_t>();
            auto [_, ok] = p_tile_data.tiles.try_emplace({ x, y }, id);
            DEV_ASSERT_MSG(ok, "duplicate tile");
        }
    }

    return true;
}

auto TileMapAsset::SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> {
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

auto TileMapAsset::LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> {
    YAML::Node root;

    if (auto res = LoadYaml(p_meta.path, root); !res) {
        return CAVE_ERROR(res.error());
    }

    YamlDeserializer deserializer;
    deserializer.Initialize(root);

    const int version = deserializer.GetVersion();

    if (deserializer.TryEnterKey("content")) {
        switch (version) {
            case 1:
                [[fallthrough]];
            default:
                deserializer.Read(*this);
                break;
        }

        deserializer.LeaveKey();
    }

    SetSpriteGuid(m_sprite_guid, true);
    return Result<void>();
}

}  // namespace cave
