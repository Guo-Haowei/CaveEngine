#include "SpriteAnimationAsset.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/serialization/YamlInclude.h"

namespace cave {

SpriteAnimationClip::SpriteAnimationClip(std::vector<math::Box2>&& frames, float length) {
    frames_ = std::move(frames);
    setAnimationLength(length);
}

void SpriteAnimationClip::setFrames(std::vector<math::Box2>&& frames) {
    frames_ = std::move(frames);
}

void SpriteAnimationClip::setAnimationLength(float length) {
    DEV_ASSERT(length > 0.0f);
    const float frame_duration = length / std::max(1, static_cast<int>(frames_.size()));  // avoid divide by 0
    durations_.resize(frames_.size());
    for (float& duration : durations_) {
        duration = frame_duration;
    }
}

bool SpriteAnimationAsset::addClip(std::string&& name, std::vector<math::Box2>&& frames) {
    auto it = clips_.find(name);
    if (it != clips_.end()) {
        LOG_WARN("clip '{}' already exists", name);
        return false;
    }

    clips_.insert(std::make_pair(std::move(name), SpriteAnimationClip(std::move(frames))));
    return true;
}

const SpriteAnimationClip* SpriteAnimationAsset::tryGetClip(const std::string& name) {
    auto it = clips_.find(name);
    if (it == clips_.end()) {
        return nullptr;
    }

    return &(it->second);
}

void SpriteAnimationAsset::SetGuid(const Guid& guid) {
    AssetHandle::replaceGuidAndHandle(AssetType::Image,
                                      guid,
                                      image_guid_,
                                      image_handle_.rawHandle());
}

void SpriteAnimationAsset::OnDeserialized() {
    auto handle = AssetRegistry::singleton().findByGuid<ImageAsset>(image_guid_);
    if (handle.is_some()) {
        image_handle_ = handle.unwrap_unchecked();
    }

    for (auto& it : clips_) {
        float& total = it.second.total_duration_;
        total = 0.0f;
        for (float duration : it.second.durations_) {
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
    deserializer.Initialize(root);

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

    OnDeserialized();
    return Result<void>();
}

}  // namespace cave
