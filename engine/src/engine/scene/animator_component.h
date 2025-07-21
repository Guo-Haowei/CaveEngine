#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/reflection/reflection.h"

namespace cave {

class Archive;

// @TODO: reuse AnimatorComponent for both sprite and skeleton
class AnimatorComponent {
    CAVE_META(AnimatorComponent)

    CAVE_PROP(editor = Asset)
    Guid m_anim_id;

    // @TODO: drop down?
    CAVE_PROP()
    std::string m_current_clip = "idle";

    CAVE_PROP(editor = Toggle)
    bool m_looping = true;

    CAVE_PROP(editor = Toggle)
    bool m_playing = true;

    // @TODO: add play speed
    CAVE_PROP()
    float m_speed = 1.0f;

    // Non-serialized

    struct {
        mutable float timer{ 0 };
        float start{ 0 };
        float end{ 0 };
    } m_playback_timer{};

    Handle<SpriteAnimationAsset> m_anim_handle;

public:
    void SetClip(const std::string& p_name, bool p_looping, float p_duration);
    const std::string& GetCurrentClip() const { return m_current_clip; }

    void SetResourceGuid(const Guid& p_guid);
    const Guid& GetResourceGuid() const { return m_anim_id; }

    const Handle<SpriteAnimationAsset>& GetAnimHandle() { return m_anim_handle; }

    void SetPlaying(bool p_playing = true) { m_playing = p_playing; }
    bool IsPlaying() const { return m_playing; }

    void SetLooping(bool p_looping = true) { m_looping = p_looping; }
    bool IsLooping() const { return m_looping; }

    const auto& GetPlaybackTimer() const { return m_playback_timer; }

    void OnDeserialized();

    void Serialize(Archive& p_archive, uint32_t p_version);
};

}  // namespace cave
