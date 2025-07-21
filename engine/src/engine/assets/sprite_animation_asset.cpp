#include "sprite_animation_asset.h"

#include "engine/assets/image_asset.h"
#include "engine/runtime/asset_registry.h"
#include "engine/serialization/yaml_include.h"

namespace cave {

SpriteAnimationClip::SpriteAnimationClip(std::vector<Rect>&& p_frames, float p_length) {
    m_frames = std::move(p_frames);
    SetAnimationLength(p_length);
}

void SpriteAnimationClip::SetFrames(std::vector<Rect>&& frames) {
    m_frames = std::move(frames);
}

void SpriteAnimationClip::SetAnimationLength(float p_length) {
    DEV_ASSERT(p_length > 0.0f);
    const float frame_duration = p_length / std::max(1, static_cast<int>(m_frames.size()));  // avoid divide by 0
    m_durations.resize(m_frames.size());
    for (float& duration : m_durations) {
        duration = frame_duration;
    }
}

bool SpriteAnimationAsset::AddClip(std::string&& p_name, std::vector<Rect>&& p_frames) {
    auto it = m_clips.find(p_name);
    if (it != m_clips.end()) {
        LOG_WARN("clip '{}' already exists", p_name);
        return false;
    }

    m_clips.insert(std::make_pair(std::move(p_name),
                                  SpriteAnimationClip(std::move(p_frames))));

    return true;
}

const SpriteAnimationClip* SpriteAnimationAsset::GetClip(const std::string& p_name) {
    auto it = m_clips.find(p_name);
    if (it == m_clips.end()) {
        return nullptr;
    }

    return &(it->second);
}

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

void SpriteAnimationAsset::OnDeserialized() {
    auto handle = AssetRegistry::GetSingleton().FindByGuid<ImageAsset>(m_image_guid);
    if (handle.is_some()) {
        m_image_handle = handle.unwrap_unchecked();
    }

    for (auto& it : m_clips) {
        float& total = it.second.m_total_duration;
        total = 0.0f;
        for (float duration : it.second.m_durations) {
            total += duration;
        }
    }
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

    OnDeserialized();
    return Result<void>();
}

}  // namespace cave
