#include "material_asset.h"

#include "engine/serialization/yaml_include.h"

namespace cave {

static MaterialAsset s_default = {};

const MaterialAsset* MaterialAsset::Default() {
    return &s_default;
}

std::vector<Guid> MaterialAsset::GetDependencies() const {
    std::vector<Guid> dependencies;
    dependencies.reserve(textures.size());
    for (const auto& texture : textures) {
        if (!texture.image_id.IsNull()) {
            dependencies.push_back(texture.image_id);
        }
    }
    return dependencies;
}

Result<void> MaterialAsset::SaveToDisk(const AssetMetaData& p_meta) const {
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

Result<void> MaterialAsset::LoadFromDisk(const AssetMetaData& p_meta) {
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

    OnDeserialized();
    return Result<void>();
}

void MaterialAsset::OnDeserialized() {
#if 0
    for (int i = 0; i < TEXTURE_MAX; ++i) {
        const auto& path = textures[i].path;
        if (!path.empty()) {
            DEV_ASSERT(0);
            // AssetRegistry::GetSingleton().RequestAssetAsync(path);
        }
    }
#endif
}

#if 0
void MaterialAsset::Serialize(Archive& p_archive, uint32_t p_version) {
    unused(p_version);

    p_archive.ArchiveValue(metallic);
    p_archive.ArchiveValue(roughness);
    p_archive.ArchiveValue(emissive);
    p_archive.ArchiveValue(baseColor);

    // @TODO: refactor this
    if (p_archive.IsWriteMode()) {
        for (int i = 0; i < TEXTURE_MAX; ++i) {
            p_archive << textures[i].enabled;
            p_archive << textures[i].path;
        }
    } else {
        for (int i = 0; i < TEXTURE_MAX; ++i) {
            p_archive >> textures[i].enabled;
            std::string& path = textures[i].path;
            p_archive >> path;
        }
    }
}
#endif

}  // namespace cave
