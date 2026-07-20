#include "SpriteAnimationAsset.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/serialization/YamlInclude.h"

namespace cave {

SpriteAnimationClip::SpriteAnimationClip(Vector<math::Box2>&& frames, float length) {
    m_frames = std::move(frames);
    setAnimationLength(length);
}

void SpriteAnimationClip::setFrames(Vector<math::Box2>&& frames) {
    m_frames = std::move(frames);
}

void SpriteAnimationClip::setAnimationLength(float length) {
    DEV_ASSERT(length > 0.0f);
    const float frame_duration = length / std::max(1, static_cast<int>(m_frames.size()));  // avoid divide by 0
    m_durations.resize(m_frames.size());
    for (float& duration : m_durations) {
        duration = frame_duration;
    }
}

bool SpriteAnimationAsset::addClip(String&& name, Vector<math::Box2>&& frames) {
    auto it = m_clips.find(name);
    if (it != m_clips.end()) {
        LOG_WARN("clip '{}' already exists", name);
        return false;
    }

    m_clips.insert(std::make_pair(std::move(name), SpriteAnimationClip(std::move(frames))));
    return true;
}

const SpriteAnimationClip* SpriteAnimationAsset::tryGetClip(const std::string& name) {
    auto it = m_clips.find(name);
    if (it == m_clips.end()) {
        return nullptr;
    }

    return &(it->second);
}

void SpriteAnimationAsset::SetGuid(const Guid& guid) {
    AssetHandle::replaceGuidAndHandle(AssetType::Image,
                                      guid,
                                      m_image_guid,
                                      m_image_handle.rawHandle());
}

void SpriteAnimationAsset::onDeserialized() {
    auto handle = AssetRegistry::singleton().findByGuid<ImageAsset>(m_image_guid);
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

auto SpriteAnimationAsset::saveToDisk(const AssetMetaData& meta) const -> Result<void> {
    auto res = meta.saveToDisk(this);
    if (!res) {
        return CAVE_ERROR(res.error());
    }

    YamlSerializer yaml;
    yaml.beginMap(false)
        .beginKey("version")
        .write(kVersion)
        .beginKey("content")
        .write(*this)
        .endMap();
    return SaveYaml(meta.import_path, yaml);
}

auto SpriteAnimationAsset::loadFromDisk(const AssetMetaData& meta) -> Result<void> {
    YAML::Node root;

    if (auto res = LoadYaml(meta.import_path, root); !res) {
        return CAVE_ERROR(res.error());
    }

    YamlDeserializer deserializer;
    deserializer.initialize(root);

    const int version = deserializer.version();

    if (deserializer.tryEnterKey("content")) {
        switch (version) {
            case 1:
                [[fallthrough]];
            default:
                deserializer.read(*this);
                break;
        }

        deserializer.leaveKey();
    }

    onDeserialized();
    return Result<void>();
}

}  // namespace cave
