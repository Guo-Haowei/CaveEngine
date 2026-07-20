#pragma once
#include <span>

#include "cave/core/math/Box.h"
#include "cave/core/reflection/Reflection.h"
#include "cave/runtime/assets/AssetHandle.h"

namespace cave {

class SpriteAnimationClip {
    CAVE_META(SpriteAnimationClip)

private:
    CAVE_PROP()
    Vector<math::Box2> m_frames;

    CAVE_PROP()
    Vector<float> m_durations;

    CAVE_PROP()
    bool m_looping = true;

    float m_total_duration = 1.0f;

public:
    SpriteAnimationClip() = default;

    SpriteAnimationClip(Vector<math::Box2>&& frames, float length = 1.0f);

    bool looping() const { return m_looping; }

    void setFrames(Vector<math::Box2>&& frames);

    void setAnimationLength(float length);

    float totalDuration() const { return m_total_duration; }

    std::span<const math::Box2> frames() const { return m_frames; }

    std::span<const float> durations() const { return m_durations; }

    friend class SpriteAnimationAsset;
};

class SpriteAnimationAsset : public IAsset {
    CAVE_ASSET(SpriteAnimationAsset, AssetType::SpriteAnimation, 0)

    CAVE_META(SpriteAnimationAsset)

private:
    CAVE_PROP(editor = Asset)
    Guid m_image_guid;

    CAVE_PROP()
    Map<String, SpriteAnimationClip> m_clips;

    // Non serialized
    Handle<ImageAsset> m_image_handle;

public:
    bool addClip(String&& name, Vector<math::Box2>&& frames);

    const SpriteAnimationClip* tryGetClip(const std::string& name);

    const auto& clips() const { return m_clips; }

    void SetGuid(const Guid& guid);

    const Guid& imageGuid() const { return m_image_guid; }

    Handle<ImageAsset> imageHandle() const { return m_image_handle; }

    auto saveToDisk(const AssetMetaData& meta) const -> Result<void> override;

    auto loadFromDisk(const AssetMetaData& meta) -> Result<void> override;

    Vector<Guid> dependencies() const override {
        return { m_image_guid };
    }

    void onDeserialized();
};

}  // namespace cave
