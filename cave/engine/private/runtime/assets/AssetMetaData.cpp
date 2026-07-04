#include "cave/core/string/StringUtils.h"
#include "cave/runtime/assets/AssetMetaData.h"
#include "cave/runtime/assets/IAsset.h"

#include "engine/private/core/io/file_access.h"
#include "engine/private/serialization/yaml_include.h"

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

    // meta sys path
    std::string sys_path = FileAccess::FixPath(FileAccess::ACCESS_RESOURCE, p_path);
    if (meta.source_created_time.empty()) {
        meta.source_created_time = std::format("{:%Y-%m-%d %H:%M:%S}", std::chrono::system_clock::now());
    }

    // asset sys path
    sys_path.resize(sys_path.size() - 5);  // remove '.meta'
    if (fs::exists(sys_path)) {
        auto ftime = fs::last_write_time(sys_path);
        auto sctp = std::chrono::clock_cast<std::chrono::system_clock>(ftime);
        meta.source_last_modified = std::format("{:%Y-%m-%d %H:%M:%S}", sctp);
    }

    return meta;
}

Result<void> AssetMetaData::saveToDisk(const IAsset* asset) const {
    YamlSerializer yaml;

    std::string asset_name = name;
    if (asset_name.empty()) {
        asset_name = StringUtils::fileName(import_path.c_str(), '/');
    }

    if (asset) {
        dependencies = asset->dependencies();
    }

    yaml.Write(*this);
    auto meta_path = std::format("{}.meta", import_path);
    return SaveYaml(meta_path, yaml);
}

auto AssetMetaData::CreateMeta(std::string_view path) -> Option<AssetMetaData> {
    auto extension = StringUtils::extension(path);

    // @TODO: [SCRUM-222] refactor this part
    AssetType type = AssetType::Blob;
    if (extension == ".png" || extension == ".jpg" || extension == ".hdr") {
        type = AssetType::Image;
    } else if (extension == ".ttf" || extension == ".lua") {
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
    } else {
        return None();
    }

    AssetMetaData meta;
    meta.guid = Guid::make();
    meta.type = type;
    meta.import_path = path;

    return Some(meta);
}

}  // namespace cave
