#include "animation_system.h"

#include "engine/assets/sprite_animation_asset.h"
#include "engine/scene/scene.h"

namespace cave {

class AnimationSystem {
public:
    static int GetFrame(float p_timer,
                        float p_total,
                        const std::vector<float>& p_durations) {
        const int frame_count = static_cast<int>(p_durations.size());
        if (p_timer >= p_total) {
            return frame_count - 1;
        }
        float time_so_far = 0.0f;
        for (int i = 0; i < frame_count; ++i) {
            time_so_far += p_durations[i];
            if (p_timer <= time_so_far) {
                return i;
            }
        }
        CRASH_NOW_MSG("should not reach here");
        return 0;
    }

    static void UpdateSpriteAnimation(float p_timestep,
                                      SpriteAnimatorComponent& p_animator,
                                      SpriteRendererComponent& p_renderer) {
        SpriteAnimationAsset* asset = p_animator.GetAnimHandle().Get();
        if (!asset) {
            return;
        }

        p_renderer.SetResourceGuid(asset->GetImageGuid());

        const auto& clip_name = p_animator.GetCurrentClip();
        const auto& clips = asset->GetClips();
        auto it = clips.find(clip_name);
        if (it == clips.end()) {
            return;
        }
        const SpriteAnimationClip& clip = it->second;

        auto& timer = p_animator.GetPlaybackTimerRef();
        if (p_animator.IsPlaying()) {
            timer += p_timestep;
        }

        const float duration = clip.GetTotalDuration();
        if (p_animator.IsLooping()) {
            timer = std::fmod(timer, duration);
        } else {
            timer = std::min(timer, duration);
        }

        const int frame_idx = GetFrame(timer,
                                       duration,
                                       clip.GetDurations());

        DEV_ASSERT_INDEX(frame_idx, clip.GetFrames().size());
        p_renderer.SetRect(clip.GetFrames()[frame_idx]);
    }
};

void RunSpriteAnimationSystem(Scene& p_scene, jobsystem::Context& p_context, float p_timestep) {
    unused(p_context);

    auto view = p_scene.View<SpriteAnimatorComponent, SpriteRendererComponent>();

    for (auto [id, animator, renderer] : view) {
        AnimationSystem::UpdateSpriteAnimation(p_timestep, animator, renderer);
    }
}

}  // namespace cave
