#include "asset_meta_data.h"

#include "engine/assets/assets.h"
#include "engine/core/io/file_access.h"
#include "engine/core/string/string_utils.h"
#include "engine/systems/serialization/serialization.h"

namespace cave {

namespace fs = std::filesystem;

auto AssetMetaData::LoadMeta(std::string_view p_path) -> Result<AssetMetaData> {
    YAML::Node node;
    if (auto res = serialize::LoadYaml(p_path, node); !res) {
        return CAVE_ERROR(res.error());
    }

    AssetMetaData meta;
    {
        auto guid = node["guid"].as<std::string>();
        auto res = Guid::Parse(guid);
        if (!res) {
            return CAVE_ERROR(res.error());
        }
        meta.guid = *res;
    }
    {
        auto type = node["type"].as<std::string>();
        meta.type = AssetTypeFromString(type);
        if (meta.type == AssetType::Unknown) {
            return CAVE_ERROR(ErrorCode::ERR_INVALID_DATA, "unknown asset type '{}'", type);
        }
    }
    meta.path = node["path"].as<std::string>();
    return meta;
}

auto AssetMetaData::CreateMeta(std::string_view p_path) -> std::optional<AssetMetaData> {
    auto extension = StringUtils::Extension(p_path);

    AssetType type = AssetType::Binary;
    if (extension == ".png" || extension == ".jpg" || extension == ".hdr") {
        type = AssetType::Image;
    } else if (extension == ".ttf") {
        type = AssetType::Binary;
    } else if (extension == ".sprite") {
        type = AssetType::Sprite;
    } else if (extension == ".tilemap") {
        type = AssetType::TileMap;
    } else {
        return std::nullopt;
    }

    AssetMetaData meta;
    meta.guid = Guid::Create();
    meta.type = type;
    meta.path = p_path;

    return meta;
}

auto AssetMetaData::SaveToDisk(const IAsset* p_asset) const -> Result<void> {
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "guid" << YAML::Value << guid.ToString();
    out << YAML::Key << "type" << YAML::Value << ToString(type);
    out << YAML::Key << "path" << YAML::Value << path;
    if (p_asset) {
        out << YAML::Key << "dependencies" << YAML::Value << YAML::BeginSeq;
        for (const Guid& id : p_asset->GetDependencies()) {
            out << id.ToString();
        }
        out << YAML::EndSeq;

        // @TODO: extra params
    }
    out << YAML::EndMap;

    auto meta_path = std::format("{}.meta", path);
    return serialize::SaveYaml(meta_path, out);
}

}  // namespace cave
