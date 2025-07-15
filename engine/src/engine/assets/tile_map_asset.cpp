#include "tile_map_asset.h"

#include "engine/assets/assets.h"
#include "engine/core/io/file_access.h"
#include "engine/runtime/asset_registry.h"
#include "engine/serialization/serialization.h"

namespace my {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TileIndex, x, y);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TileData, id);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TileMapLayer, name, sprite_guid, tiles);

TileData TileMapLayer::GetTile(TileIndex p_index) {
    auto it = tiles.find(p_index);
    if (it == tiles.end()) {
        return TileData::Empty();
    }

    return it->second;
}

TileResult TileMapLayer::SetTile(TileIndex p_index, TileData p_new_tile, TileData& p_out_old) {
    p_out_old = TileData::Empty();

    auto it = tiles.find(p_index);

    // if there's a tile
    if (it != tiles.end()) {
        p_out_old = it->second;  // save the old tile

        if (p_new_tile.IsEmpty()) {
            // do nothing, the tile was already empty
            tiles.erase(it);

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
    if (p_new_tile.IsEmpty()) {
        // can't replace an empty tile with an empty tile
        return TileResult::Noop;
    }

    tiles.insert({ p_index, p_new_tile });
    return TileResult::Added;
}

TileMapLayer& TileMapAsset::AddLayer(std::string&& p_name) {
    m_layers.resize(m_layers.size() + 1);

    auto& layer = m_layers.back();
    layer.name = std::move(p_name);

    return layer;
}

std::vector<Guid> TileMapAsset::GetDependencies() const {
    std::vector<Guid> dependencies;
    dependencies.reserve(m_layers.size());
    for (const auto& layer : m_layers) {
        dependencies.push_back(layer.sprite_guid);
    }
    return dependencies;
}

auto TileMapAsset::SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> {
    // meta
    auto res = p_meta.SaveToDisk(this);
    if (!res) {
        return HBN_ERROR(res.error());
    }

    json j;
    j["version"] = VERSION;
    j["layers"] = m_layers;

    return Serialize(p_meta.path, j);
}

void TileMapAsset::LoadFromDiskCurrent(const json& j) {
    m_layers = j.at("layers").get<std::vector<TileMapLayer>>();
}

auto TileMapAsset::LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> {
    json j;

    if (auto res = Deserialize(p_meta.path, j); !res) {
        return HBN_ERROR(res.error());
    }

    const int version = j["version"];

    try {
        switch (version) {
            case 1:
                [[fallthrough]];
            default:
                LoadFromDiskCurrent(j);
                break;
        }
    } catch (const json::exception& e) {
        return HBN_ERROR(ErrorCode::ERR_PARSE_ERROR, "{}", e.what());
    }

    return Result<void>();
}

#if 0
void SpriteSheetAsset::UpdateFrames() {
    DEV_ASSERT(m_row > 0 && m_column > 0);
    m_frames.clear();
    m_frames.reserve(m_row * m_column);

    const float dx = static_cast<float>(m_width) / m_column;
    const float dy = static_cast<float>(m_height) / m_row;

    for (uint32_t y = 0; y < m_row; ++y) {
        for (uint32_t x = 0; x < m_column; ++x) {
            const float x_min = x * dx;
            const float y_min = y * dy;
            const float x_max = (x + 1) * dx;
            const float y_max = (y + 1) * dy;

            m_frames.push_back(Rect({ x_min, y_min }, { x_max, y_max }));
        }
    }
}
#endif

}  // namespace my
