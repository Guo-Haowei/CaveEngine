#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/math/box.h"
#include "engine/reflection/reflection.h"

namespace cave {

class SpriteAnimationClip {
    CAVE_META(SpriteAnimationClip)

    CAVE_PROP(type = box2[])
    std::vector<Rect> m_frames;

    CAVE_PROP(type = f32[])
    std::vector<float> m_durations;

    CAVE_PROP(type = boolean, hint = toggle)
    bool m_loop = true;

    float m_total_duration = 1.0f;

public:
    SpriteAnimationClip() = default;

    SpriteAnimationClip(std::vector<Rect>&& p_frames, float p_length = 1.0f);

    bool IsLooping() const { return m_loop; }

    void SetFrames(std::vector<Rect>&& frames);

    void SetAnimationLength(float p_length);

    float GetTotalDuration() const { return m_total_duration; }

    const std::vector<Rect>& GetFrames() const { return m_frames; }

    const std::vector<float>& GetDurations() const { return m_durations; }

    friend class SpriteAnimationAsset;
};

class SpriteAnimationAsset : public IAsset {
    CAVE_ASSET(SpriteAnimationAsset, AssetType::SpriteAnimation, 0)

    CAVE_META(SpriteAnimationAsset)

    CAVE_PROP(type = guid, tooltip = "image id")
    Guid m_image_guid;

    CAVE_PROP()
    std::map<std::string, SpriteAnimationClip> m_clips;

    // Non serialized
    Handle<ImageAsset> m_image_handle;

public:
    bool AddClip(std::string&& p_name, std::vector<Rect>&& p_frames);

    const SpriteAnimationClip* GetClip(const std::string& p_name);

    const auto& GetClips() const { return m_clips; }

    void SetGuid(const Guid& p_guid);

    const Guid& GetImageGuid() const { return m_image_guid; }

    Handle<ImageAsset> GetImageHandle() const { return m_image_handle; }

    auto SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> override;

    auto LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> override;

    std::vector<Guid> GetDependencies() const override {
        return { m_image_guid };
    }

    void OnDeserialized();
};

}  // namespace cave
