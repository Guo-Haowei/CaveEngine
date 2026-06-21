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
    Guid anim_id_;

    // @TODO: drop down?
    CAVE_PROP()
    std::string current_clip_ = "idle";

    CAVE_PROP(editor = Toggle)
    bool looping_ = true;

    CAVE_PROP(editor = Toggle)
    bool playing_ = true;

    // @TODO: add play speed
    CAVE_PROP()
    float speed_ = 1.0f;

    // Non-serialized
    float playback_timer_{ 0 };

    Handle<SpriteAnimationAsset> anim_handle_;

public:
    void currentClip(std::string_view name);
    const std::string& currentClip() const { return current_clip_; }

    bool SetResourceGuid(const Guid& guid);
    const Guid& GetResourceGuid() const { return anim_id_; }

    const Handle<SpriteAnimationAsset>& animHandle() { return anim_handle_; }

    void playing(bool playing) { playing_ = playing; }
    bool playing() const { return playing_; }

    void looping(bool looping) { looping_ = looping; }
    bool looping() const { return looping_; }

    void playbackTimer(float timer) { playback_timer_ = timer; }
    float playbackTimer() const { return playback_timer_; }

    void OnDeserialized();
};

}  // namespace cave
