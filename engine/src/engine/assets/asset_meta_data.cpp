#include "asset_meta_data.h"

#include "engine/assets/assets.h"
#include "engine/core/io/file_access.h"
#include "engine/core/string/string_utils.h"
#include "engine/systems/serialization/serialization.h"

namespace my {

namespace fs = std::filesystem;

const char* AssetType::ToString() const {
    static constexpr const char* s_table[] = {
#define ASSET_TYPE(ENUM, NAME) NAME,
        ASSET_TYPE_LIST
#undef ASSET_TYPE
    };
    static_assert(array_length(s_table) == std::to_underlying(AssetType::Count));

    return s_table[std::to_underlying(m_type)];
}

static Result<AssetType> ParseAssetType(std::string_view p_string) {
#define ASSET_TYPE(ENUM, NAME) \
    if (p_string == NAME) { return AssetType::ENUM; }
    ASSET_TYPE_LIST
#undef ASSET_TYPE

    return HBN_ERROR(ErrorCode::ERR_INVALID_DATA, "invalid type {}", p_string);
}

auto AssetMetaData::LoadMeta(std::string_view p_path) -> Result<AssetMetaData> {
    YAML::Node node;
    if (auto res = serialize::LoadYaml(p_path, node); !res) {
        return HBN_ERROR(res.error());
    }

    AssetMetaData meta;
    {
        auto guid = node["guid"].as<std::string>();
        auto res = Guid::Parse(guid);
        if (!res) {
            return HBN_ERROR(res.error());
        }
        meta.guid = *res;
    }
    {
        auto type = node["type"].as<std::string>();
        auto res = ParseAssetType(type);
        if (!res) {
            return HBN_ERROR(res.error());
        }
        meta.type = *res;
    }
    meta.path = node["path"].as<std::string>();
    return meta;
}

auto AssetMetaData::CreateMeta(std::string_view p_path) -> std::optional<AssetMetaData> {
    auto extension = StringUtils::Extension(p_path);

    AssetType type = AssetType::Binary;
    if (extension == ".png" || extension == ".jpg") {
        type = AssetType::Image;
    } else if (extension == ".ttf") {
        type = AssetType::Binary;
    } else if (extension == ".sprite") {
        type = AssetType::SpriteSheet;
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
    out << YAML::Key << "type" << YAML::Value << type.ToString();
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

}  // namespace my
