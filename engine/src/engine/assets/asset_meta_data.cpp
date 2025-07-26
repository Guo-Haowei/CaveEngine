#include "asset_meta_data.h"

#include "engine/assets/asset_interface.h"
#include "engine/core/io/file_access.h"
#include "engine/core/string/string_utils.h"
#include "engine/serialization/yaml_include.h"

namespace cave {

namespace fs = std::filesystem;

auto AssetMetaData::LoadMeta(std::string_view p_path) -> Result<AssetMetaData> {
    YAML::Node root;
    if (auto res = LoadYaml(p_path, root); !res) {
        return CAVE_ERROR(res.error());
    }

    AssetMetaData meta;

    YamlDeserializer d;
    d.Initialize(root);
    d.Read(meta);
    return meta;
}

Result<void> AssetMetaData::SaveToDisk(const IAsset* p_asset) const {
    YamlSerializer yaml;

    std::string asset_name = name;
    if (asset_name.empty()) {
        asset_name = StringUtils::FileName(import_path.c_str(), '/');
    }

    this->dependencies = p_asset->GetDependencies();

    yaml.Write(*this);
    auto meta_path = std::format("{}.meta", import_path);
    return SaveYaml(meta_path, yaml);
}

auto AssetMetaData::CreateMeta(std::string_view p_path) -> Option<AssetMetaData> {
    auto extension = StringUtils::Extension(p_path);

    // @TODO: [SCRUM-222] refactor this part
    AssetType type = AssetType::Blob;
    if (extension == ".png" || extension == ".jpg" || extension == ".hdr") {
        type = AssetType::Image;
    } else if (extension == ".ttf") {
        type = AssetType::Blob;
    } else if (extension == ".lua") {
        type = AssetType::Blob;
    } else if (extension == ".tileset") {
        type = AssetType::TileSet;
    } else if (extension == ".tilemap") {
        type = AssetType::TileMap;
    } else if (extension == ".sprite_anim") {
        type = AssetType::SpriteAnimation;
    } else if (extension == ".scene") {
        type = AssetType::Scene;
    } else if (extension == ".mat") {
        type = AssetType::Material;
    } else if (extension == ".obj") {
        type = AssetType::Mesh;
    } else {
        return None();
    }

    AssetMetaData meta;
    meta.guid = Guid::Create();
    meta.type = type;
    meta.import_path = p_path;

    return Some(meta);
}

}  // namespace cave
