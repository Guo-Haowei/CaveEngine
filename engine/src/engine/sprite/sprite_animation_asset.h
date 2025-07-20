#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/assets/asset_interface.h"
#include "engine/math/box.h"
#include "engine/reflection/reflection.h"

namespace cave {

struct SpriteAnimationClip {
    CAVE_META(SpriteAnimationClip)

    CAVE_PROP(type = name)
    std::string name;

    CAVE_PROP(type = rect[])
    std::vector<Rect> frames;

    CAVE_PROP(type = f32[])
    std::vector<float> durations;

    CAVE_PROP(type = boolean)
    bool loop = true;
};

class SpriteAnimationAsset : public IAsset {
    CAVE_ASSET(SpriteAnimationAsset, AssetType::SpriteAnimation, 0)

    CAVE_META(SpriteAnimationAsset)

    CAVE_PROP(type = guid, tooltip = "image id")
    Guid m_image_guid;

    CAVE_PROP()
    std::unordered_map<std::string, SpriteAnimationClip> clips;

private:
    // Non serialized
    Handle<ImageAsset> m_image_handle;

public:
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
