// =============================================================================
// File: cave/runtime/ecs/components/SpriteAnimatorComponent.h
// =============================================================================
#pragma once
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class SpriteAnimatorComponent {
    CAVE_COMPONENT(SpriteAnimatorComponent)

private:
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
    float m_playback_timer{ 0 };

    Handle<SpriteAnimationAsset> m_anim_handle;

public:
    void currentClip(std::string_view name);
    const std::string& currentClip() const { return m_current_clip; }

    bool setAnimGuid(const Guid& guid);
    const Guid& animGuid() const { return m_anim_id; }

    const Handle<SpriteAnimationAsset>& animHandle() { return m_anim_handle; }

    void play() { m_playing = true; }
    void pause() { m_playing = false; }

    bool playing() const { return m_playing; }

    void setLooping(bool looping) { m_looping = looping; }
    bool looping() const { return m_looping; }

    void playbackTimer(float timer) { m_playback_timer = timer; }
    float playbackTimer() const { return m_playback_timer; }

    void onDeserialized();
};

}  // namespace cave
