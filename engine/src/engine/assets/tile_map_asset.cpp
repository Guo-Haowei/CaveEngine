#include "tile_map_asset.h"

#include "engine/assets/assets.h"
#include "engine/core/io/file_access.h"
#include "engine/runtime/asset_registry.h"
#include "engine/systems/serialization/serialization.h"

namespace my {

TileData TileMapLayer::GetTile(TileIndex p_index) {
    auto it = m_tiles.find(p_index);
    if (it == m_tiles.end()) {
        return TileData::Empty();
    }

    return it->second;
}

TileResult TileMapLayer::SetTile(TileIndex p_index, TileData p_new_tile, TileData& p_out_old) {
    p_out_old = TileData::Empty();

    auto it = m_tiles.find(p_index);

    // if there's a tile
    if (it != m_tiles.end()) {
        p_out_old = it->second;  // save the old tile

        if (p_new_tile.IsEmpty()) {
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
    if (p_new_tile.IsEmpty()) {
        // can't replace an empty tile with an empty tile
        return TileResult::Noop;
    }

    m_tiles.insert({ p_index, p_new_tile });
    return TileResult::Added;
}

TileMapLayer& TileMapAsset::AddLayer(std::string&& p_name) {
    m_layers.resize(m_layers.size() + 1);

    auto& layer = m_layers.back();
    layer.name = std::move(p_name);

    return layer;
}

auto TileMapLayer::FromYaml(const YAML::Node& p_node) -> Result<void> {
    unused(p_node);
    return Result<void>();
}

auto TileMapLayer::ToYaml(YAML::Emitter& p_out) const -> Result<void> {
    // serialize::SerializeYamlContext ctx;

    p_out << YAML::BeginMap;

    p_out << YAML::Key << "name" << YAML::Value << name;
    p_out << YAML::Key << "tiles" << YAML::Value << YAML::BeginSeq;

    p_out.SetMapFormat(YAML::Flow);

    for (const auto& [key, value] : m_tiles) {
        p_out << YAML::BeginMap;
        p_out << YAML::Key << "x" << YAML::Value << key.x;
        p_out << YAML::Key << "y" << YAML::Value << key.y;
        p_out << YAML::Key << "type" << YAML::Value << value.id;
        p_out << YAML::EndMap;
    }

    p_out << YAML::EndSeq;

    p_out << YAML::EndMap;
    return Result<void>();
}

std::vector<Guid> TileMapAsset::GetDependencies() const {
    return {};
}

auto TileMapAsset::SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> {
    // meta
    auto res = p_meta.SaveToDisk(this);
    if (!res) {
        return HBN_ERROR(res.error());
    }

    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "version" << YAML::Value << VERSION;

    out << YAML::Key << "body" << YAML::Value;

    serialize::SerializeYamlContext context;
    serialize::SerializeYaml(out, *this, context);

    out << YAML::EndMap;
    return serialize::SaveYaml(p_meta.path, out);
}

auto TileMapAsset::LoadFromDiskCurrent(const YAML::Node& p_node) -> Result<void> {
    serialize::SerializeYamlContext context;
    return serialize::DeserializeYaml(p_node, *this, context);
}

auto TileMapAsset::LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> {
    YAML::Node node;
    if (auto res = serialize::LoadYaml(p_meta.path, node); !res) {
        return HBN_ERROR(res.error());
    }

    const auto& version_node = node["version"];
    const int version = version_node ? version_node.as<int>() : 0;
    auto body = node["body"];

    switch (version) {
        case 1:
            [[fallthrough]];
        default:
            LoadFromDiskCurrent(body);
            break;
    }

    return Result<void>();
}

void TileMapLayer::RegisterClass() {
    BEGIN_REGISTRY(TileMapLayer);
    REGISTER_FIELD(TileMapLayer, "name", name);
    REGISTER_FIELD(TileMapLayer, "guid", guid);
    END_REGISTRY(TileMapLayer);
}

void TileMapAsset::RegisterClass() {
    BEGIN_REGISTRY(TileMapAsset);
    REGISTER_FIELD(TileMapAsset, "layers", m_layers);
    END_REGISTRY(TileMapAsset);
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
