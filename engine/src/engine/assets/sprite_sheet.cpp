#include "sprite_sheet.h"

#include "engine/assets/assets.h"
#include "engine/core/io/file_access.h"
#include "engine/runtime/asset_registry.h"
#include "engine/systems/serialization/serialization.h"

namespace my {

void SpriteSheetAsset::SetSeparation(const Vector2i& p_sep) {
    if (p_sep.x < 1 || p_sep.y < 1 || p_sep == m_separation) {
        return;
    }

    m_separation = p_sep;
    // @TODO: update frames
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

        m_dimension = { image->width,
                        image->height };
    }
}

void SpriteSheetAsset::SetImage(const std::string& p_path) {
    auto handle = AssetRegistry::GetSingleton().FindByPath(p_path);
    if (handle) {
        SetHandle(std::move(*handle));
    }
}

std::vector<Guid> SpriteSheetAsset::GetDependencies() const {
    return { m_image };
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

    out << YAML::Key << "dimension" << YAML::Value;
    serialize::SerializeYaml(out, m_dimension, ctx);

    out << YAML::Key << "separation" << YAML::Value;
    serialize::SerializeYaml(out, m_separation, ctx);

    out << YAML::Key << "offset" << YAML::Value;
    serialize::SerializeYaml(out, m_offset, ctx);

    out << YAML::Key << "frames" << YAML::Value << YAML::BeginSeq;
    for (const Rect& rect : m_frames) {
        serialize::SerializeYaml(out, rect, ctx);
    }
    out << YAML::EndSeq;
    out << YAML::EndMap;

    return serialize::SaveYaml(p_meta.path, out);
}

auto SpriteSheetAsset::LoadFromDiskV0(const YAML::Node& p_node) -> Result<void> {
    serialize::SerializeYamlContext ctx;

    auto res = Guid::Parse(p_node["image_id"].as<std::string>());
    if (!res) {
        return HBN_ERROR(res.error());
    }

    m_image = *res;
    serialize::DeserializeYaml(p_node["separation"], m_separation, ctx);
    serialize::DeserializeYaml(p_node["offset"], m_offset, ctx);

    return Result<void>();
}

// @TODO: make context a serializer
auto SpriteSheetAsset::LoadFromDiskV1(const YAML::Node& p_node) -> Result<void> {
    serialize::SerializeYamlContext ctx;

    auto res = Guid::Parse(p_node["image"].as<std::string>());
    if (!res) {
        return HBN_ERROR(res.error());
    }

    m_image = *res;
    serialize::DeserializeYaml(p_node["separation"], m_separation, ctx);
    serialize::DeserializeYaml(p_node["offset"], m_offset, ctx);
    serialize::DeserializeYaml(p_node["dimension"], m_dimension, ctx);

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
            LoadFromDiskV1(node);
            break;
        default:
            LoadFromDiskV0(node);
            break;
    }

    // update image
    auto handle = AssetRegistry::GetSingleton().FindByGuid(m_image);
    if (handle) {
        SetHandle(std::move(*handle));
    }
    // @TODO: update frames

    return Result<void>();
}

}  // namespace my
