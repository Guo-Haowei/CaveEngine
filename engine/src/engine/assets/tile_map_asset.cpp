#include "tile_map_asset.h"

#include "engine/assets/assets.h"
#include "engine/core/io/file_access.h"
#include "engine/runtime/asset_registry.h"
#include "engine/systems/serialization/serialization.h"

namespace my {

#if 0
void SpriteSheetAsset::SetImage(const std::string& p_path) {
    auto handle = AssetRegistry::GetSingleton().FindByPath(p_path);
    if (handle) {
        SetHandle(std::move(*handle));
    }

    UpdateFrames();
}

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

std::vector<Guid> TileMapAsset::GetDependencies() const {
    return {};
}

auto TileMapAsset::SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> {
    // meta
    auto res = p_meta.SaveToDisk(this);
    if (!res) {
        return HBN_ERROR(res.error());
    }

    // file
    // serialize::SerializeYamlContext ctx;

    YAML::Emitter out;
    out << YAML::BeginMap;

    out << YAML::Key << "version" << YAML::Value << VERSION;

    out << YAML::EndMap;

    return serialize::SaveYaml(p_meta.path, out);
}

auto TileMapAsset::LoadFromDiskCurrent(const YAML::Node& p_node) -> Result<void> {
    // serialize::SerializeYamlContext ctx;
    unused(p_node);

    return Result<void>();
}

auto TileMapAsset::LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> {
    YAML::Node node;
    if (auto res = serialize::LoadYaml(p_meta.path, node); !res) {
        return HBN_ERROR(res.error());
    }

    const auto& version_node = node["version"];
    const int version = version_node ? version_node.as<int>() : 0;

    switch (version) {
        case 1:
            [[fallthrough]];
        default:
            LoadFromDiskCurrent(node);
            break;
    }

    return Result<void>();
}

}  // namespace my
