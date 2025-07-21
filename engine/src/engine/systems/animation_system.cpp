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
                                      AnimatorComponent& p_animator,
                                      SpriteRenderer& p_renderer) {
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

        const auto& timer = p_animator.GetPlaybackTimer();

        if (p_animator.IsPlaying()) {
            timer.timer += p_timestep;
        }

        const float duration = clip.GetTotalDuration();
        if (p_animator.IsLooping()) {
            timer.timer = std::fmod(timer.timer, duration);
        } else {
            timer.timer = std::min(timer.timer, duration);
        }

        const int frame_idx = GetFrame(timer.timer,
                                       duration,
                                       clip.GetDurations());

        DEV_ASSERT_INDEX(frame_idx, clip.GetFrames().size());
        p_renderer.rect = clip.GetFrames()[frame_idx];
    }
};

void RunSpriteAnimationSystem(Scene& p_scene, jobsystem::Context& p_context, float p_timestep) {
    // @TODO: use jobsystem
    unused(p_context);

    for (auto [id, animator] : p_scene.View<AnimatorComponent>()) {
        SpriteRenderer* renderer = p_scene.GetComponent<SpriteRenderer>(id);
        DEV_ASSERT(renderer);
        if (!renderer) continue;

        AnimationSystem::UpdateSpriteAnimation(p_timestep, animator, *renderer);
    }
}

}  // namespace cave
