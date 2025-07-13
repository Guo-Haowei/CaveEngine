#include "sprite_sheet_asset.h"

#include "engine/assets/assets.h"
#include "engine/core/io/file_access.h"
#include "engine/runtime/asset_registry.h"
#include "engine/systems/serialization/serialization.h"

namespace my {

void SpriteSheetAsset::SetRow(uint32_t p_row) {
    if (p_row == 0) return;
    if (p_row == m_row) return;
    m_row = p_row;
    UpdateFrames();
}

void SpriteSheetAsset::SetCol(uint32_t p_col) {
    if (p_col == 0) return;
    if (p_col == m_column) return;
    m_column = p_col;
    UpdateFrames();
}

void SpriteSheetAsset::SetHandle(AssetHandle&& p_handle) {
    m_image_handle = std::move(p_handle);
    const ImageAsset* image = m_image_handle.Get<ImageAsset>();
    if (image) {
        Guid guid = m_image_handle.GetGuid();
        if (guid != m_image) {
            LOG("SpriteSheetAsset: GUID changed from {} to {}", m_image.ToString(), guid.ToString());
            m_image = guid;
        }

        m_width = image->width;
        m_height = image->height;
    }
}

void SpriteSheetAsset::SetImage(const std::string& p_path) {
    auto handle = AssetRegistry::GetSingleton().FindByPath(p_path);
    if (handle) {
        SetHandle(std::move(*handle));
    }

    UpdateFrames();
}

std::vector<Guid> SpriteSheetAsset::GetDependencies() const {
    return { m_image };
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

auto SpriteSheetAsset::SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> {
    // meta
    auto res = p_meta.SaveToDisk(this);
    if (!res) {
        return HBN_ERROR(res.error());
    }

    // file
    serialize::SerializeYamlContext ctx;

    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "version" << YAML::Value << VERSION;

    out << YAML::Key << "image" << YAML::Value << m_image.ToString();

    out << YAML::Key << "width" << YAML::Value;
    serialize::SerializeYaml(out, m_width, ctx);

    out << YAML::Key << "height" << YAML::Value;
    serialize::SerializeYaml(out, m_height, ctx);

    out << YAML::Key << "row" << YAML::Value;
    serialize::SerializeYaml(out, m_row, ctx);
    out << YAML::Key << "column" << YAML::Value;
    serialize::SerializeYaml(out, m_column, ctx);

    out << YAML::EndSeq;
    out << YAML::EndMap;

    return serialize::SaveYaml(p_meta.path, out);
}

auto SpriteSheetAsset::LoadFromDiskCurrent(const YAML::Node& p_node) -> Result<void> {
    serialize::SerializeYamlContext ctx;

    auto res = Guid::Parse(p_node["image"].as<std::string>());
    if (!res) {
        return HBN_ERROR(res.error());
    }

    m_image = *res;
    serialize::DeserializeYaml(p_node["row"], m_row, ctx);
    serialize::DeserializeYaml(p_node["column"], m_column, ctx);
    serialize::DeserializeYaml(p_node["width"], m_width, ctx);
    serialize::DeserializeYaml(p_node["height"], m_height, ctx);

    return Result<void>();
}

auto SpriteSheetAsset::LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> {
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

    // update image
    auto handle = AssetRegistry::GetSingleton().FindByGuid(m_image);
    if (handle) {
        SetHandle(std::move(*handle));
    }
    UpdateFrames();

    return Result<void>();
}

}  // namespace my
