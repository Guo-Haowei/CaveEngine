#include "animation_system.h"

#include "engine/assets/sprite_animation_asset.h"
#include "engine/scene/scene.h"

namespace cave {

class AnimationSystem {
public:
    static int GetFrame(float p_current,
                        float p_total,
                        const std::vector<float>& p_durations,
                        bool p_looping) {
        const int frame_count = static_cast<int>(p_durations.size());
        if (p_current >= p_total && !p_looping) {
            return frame_count - 1;
        }
        const float timer = std::fmod(p_current, p_total);
        float time_so_far = 0.0f;
        for (int i = 0; i < frame_count; ++i) {
            time_so_far += p_durations[i];
            if (timer <= time_so_far) {
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

        p_renderer.SetImage(asset->GetImageGuid());

        const auto& clip_name = p_animator.GetCurrentClip();
        const auto& clips = asset->GetClips();
        auto it = clips.find(clip_name);
        if (it == clips.end()) {
            return;
        }

        const auto& timer = p_animator.GetPlaybackTimer();
        if (p_animator.IsPlaying()) {
            timer.timer += p_timestep;
        }

        const SpriteAnimationClip& clip = it->second;

        const int frame_idx = GetFrame(timer.timer,
                                       clip.GetTotalDuration(),
                                       clip.GetDurations(),
                                       p_animator.IsLooping());

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
