#pragma once
#include <span>

#include "cave/core/math/Box.h"
#include "cave/core/reflection/Reflection.h"
#include "cave/runtime/assets/AssetHandle.h"

namespace cave {

class SpriteAnimationClip {
    CAVE_META(SpriteAnimationClip)

private:
    CAVE_PROP(type = box2[])
    std::vector<math::Box2> frames_;

    CAVE_PROP(type = f32[])
    std::vector<float> durations_;

    CAVE_PROP(type = boolean, hint = toggle)
    bool looping_ = true;

    float total_duration_ = 1.0f;

public:
    SpriteAnimationClip() = default;

    SpriteAnimationClip(std::vector<math::Box2>&& frames, float length = 1.0f);

    bool looping() const { return looping_; }

    void setFrames(std::vector<math::Box2>&& frames);

    void setAnimationLength(float length);

    float totalDuration() const { return total_duration_; }

    std::span<const math::Box2> frames() const { return frames_; }

    std::span<const float> durations() const { return durations_; }

    friend class SpriteAnimationAsset;
};

class SpriteAnimationAsset : public IAsset {
    CAVE_ASSET(SpriteAnimationAsset, AssetType::SpriteAnimation, 0)

    CAVE_META(SpriteAnimationAsset)

private:
    CAVE_PROP(editor = Asset, tooltip = "image id")
    Guid image_guid_;

    CAVE_PROP()
    std::map<std::string, SpriteAnimationClip> clips_;

    // Non serialized
    Handle<ImageAsset> image_handle_;

public:
    bool addClip(std::string&& name, std::vector<math::Box2>&& frames);

    const SpriteAnimationClip* tryGetClip(const std::string& name);

    const auto& clips() const { return clips_; }

    void SetGuid(const Guid& guid);

    const Guid& imageGuid() const { return image_guid_; }

    Handle<ImageAsset> imageHandle() const { return image_handle_; }

    auto saveToDisk(const AssetMetaData& meta) const -> Result<void> override;

    auto loadFromDisk(const AssetMetaData& meta) -> Result<void> override;

    std::vector<Guid> dependencies() const override {
        return { image_guid_ };
    }

    void OnDeserialized();
};

}  // namespace cave
