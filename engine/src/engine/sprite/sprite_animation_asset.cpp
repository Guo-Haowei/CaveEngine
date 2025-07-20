#include "sprite_animation_asset.h"

#include "engine/serialization/yaml_include.h"

namespace cave {

void SpriteAnimationAsset::SetGuid(const Guid& p_guid) {
    AssetHandle::ReplaceGuidAndHandle(AssetType::Image,
                                      p_guid,
                                      m_image_guid,
                                      m_image_handle.RawHandle());
}

auto SpriteAnimationAsset::SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> {
    auto res = p_meta.SaveToDisk(this);
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    YamlSerializer yaml;
    yaml.BeginMap(false)
        .Key("version")
        .Write(VERSION)
        .Key("content")
        .Write(*this)
        .EndMap();
    return SaveYaml(p_meta.path, yaml);
}

auto SpriteAnimationAsset::LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> {
    YAML::Node root;

    if (auto res = LoadYaml(p_meta.path, root); !res) {
        return CAVE_ERROR(res.error());
    }

    YamlDeserializer deserializer;
    deserializer.Initialize(root);

    const int version = deserializer.GetVersion();

    if (deserializer.TryEnterKey("content")) {
        switch (version) {
            case 1:
                [[fallthrough]];
            default:
                deserializer.Read(*this);
                break;
        }

        deserializer.LeaveKey();
    }

    // SetSpriteGuid(m_sprite_guid, true);
    return Result<void>();
}

}  // namespace cave
