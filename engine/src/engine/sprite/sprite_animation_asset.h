#pragma once
#include "engine/assets/asset_interface.h"
#include "engine/math/box.h"
#include "engine/reflection/reflection.h"

namespace cave {

using Sprite = Rect;

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
    CAVE_ASSET(SpriteAnimationAsset, AssetType::SpriteAnimation)

    CAVE_META(SpriteAnimationAsset)

    CAVE_PROP(type = guid, tooltip = "image id")
    Guid m_image_guid;

    CAVE_PROP()
    std::unordered_map<std::string, SpriteAnimationClip> clips;

public:
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
