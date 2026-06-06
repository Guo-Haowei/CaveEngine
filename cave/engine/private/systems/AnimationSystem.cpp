#include "AnimationSystem.h"

#include "cave/core/diagnostics/Log.h"
#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/ecs/components/SpriteRendererComponent.h"
#include "cave/runtime/ecs/components/TransformAnimationComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

#include "engine/private/runtime/assets/SpriteAnimationAsset.h"
#include "engine/private/runtime/scene/Scene.h"

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

void RunTransformAnimationSystem(Scene& p_scene, jobsystem::Context& p_context, float p_timestep) {
    unused(p_context);

    auto view = p_scene.View<TransformAnimationComponent, TransformComponent>();

    std::vector<ecs::Entity> pending_removes;

    for (auto [id, anim, trans] : view) {
        if (!anim.playing) {
            continue;
        }
        anim.elapsed += p_timestep;
        const float t = anim.elapsed / anim.duration;
        if (t >= 1.0f) {
            trans.SetTranslation(anim.end);
            anim.playing = false;
            if (anim.destroy_on_finish) {
                pending_removes.push_back(id);
            }
            continue;
        }

        auto& p = trans.GetTranslation();
        LOG_OK("pos: {} {} {}, t: {}", p.x, p.y, p.z, t);
        math::Vector3f pos = t * anim.end + (1 - t) * anim.begin;
        trans.SetTranslation(pos);
    }

    for (ecs::Entity e : pending_removes) {
        p_scene.Remove<TransformAnimationComponent>(e);
    }
}

}  // namespace cave
