#include "asset_meta_data.h"

#include "engine/assets/assets.h"
#include "engine/core/io/file_access.h"
#include "engine/core/string/string_utils.h"
#include "engine/serialization/yaml_include.h"

namespace cave {

namespace fs = std::filesystem;

auto AssetMetaData::LoadMeta(std::string_view p_path) -> Result<AssetMetaData> {
    YAML::Node node;
    if (auto res = LoadYaml(p_path, node); !res) {
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

auto AssetMetaData::CreateMeta(std::string_view p_path) -> Option<AssetMetaData> {
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
    } else if (extension == ".sprite_anim") {
        type = AssetType::SpriteAnimation;
    } else {
        return Option<AssetMetaData>::None();
    }

    AssetMetaData meta;
    meta.guid = Guid::Create();
    meta.type = type;
    meta.path = p_path;

    return meta;
}

auto AssetMetaData::SaveToDisk(const IAsset* p_asset) const -> Result<void> {
    YamlSerializer serializer;

    // @TODO: fix this
    serializer
        .BeginMap(false)
        .Key("guid")
        .Write(guid)
        .Key("type")
        .Write(ToString(type))
        .Key("path")
        .Write(path);

    if (p_asset) {
        serializer
            .Key("dependencies")
            .Write(p_asset->GetDependencies());
    }

    serializer.EndMap();

    auto meta_path = std::format("{}.meta", path);
    return SaveYaml(meta_path, serializer);
}

}  // namespace cave
