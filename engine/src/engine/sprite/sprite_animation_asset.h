#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/assets/asset_interface.h"
#include "engine/math/box.h"
#include "engine/reflection/reflection.h"

namespace cave {

#if 0

struct SpriteAnimatorComponent {
    Handle<SpriteAnimationAsset> asset;
    std::string currentClip;
    float timeInClip = 0.0f;
    int currentFrameIndex = 0;
    bool playing = true;
};

struct SpriteRendererComponent {
    Color tint = Color::White;
    bool flipX = false;
    bool flipY = false;

    Handle<Texture> overrideTexture; // optional, usually from animation asset
    Rect uv;
    Vec2 pivot;
};

#endif

class SpriteAnimationClip {
    CAVE_META(SpriteAnimationClip)

    CAVE_PROP(type = box2[])
    std::vector<Rect> m_frames;

    CAVE_PROP(type = f32[])
    std::vector<float> m_durations;

    CAVE_PROP(type = boolean, hint = toggle)
    bool m_loop = true;

public:
    SpriteAnimationClip() = default;

    SpriteAnimationClip(std::vector<Rect>&& p_frames, float p_length = 1.0f);

    void SetFrames(std::vector<Rect>&& frames);

    void SetAnimationLength(float p_length);

    const std::vector<Rect>& GetFrames() const { return m_frames; }

    const std::vector<float>& GetDurations() const { return m_durations; }
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

    const auto& GetClips() const { return m_clips; }

    void SetGuid(const Guid& p_guid);

    const Guid& GetImageGuid() const { return m_image_guid; }

    Handle<ImageAsset> GetImageHandle() const { return m_image_handle; }

    auto SaveToDisk(const AssetMetaData& p_meta) const -> Result<void> override;

    auto LoadFromDisk(const AssetMetaData& p_meta) -> Result<void> override;

    std::vector<Guid> GetDependencies() const {
        return { m_image_guid };
    }
};

}  // namespace cave
