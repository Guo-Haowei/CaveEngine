#include "AnimationSystem.h"

#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"
#include "cave/runtime/ecs/components/SpriteRendererComponent.h"
#include "cave/runtime/ecs/components/TransformAnimationComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"

#include "engine/private/runtime/assets/SpriteAnimationAsset.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

class AnimationSystem {
public:
    static int getFrame(float timer,
                        float total,
                        std::span<const float> durations) {
        const int frame_count = static_cast<int>(durations.size());
        if (timer >= total) {
            return frame_count - 1;
        }
        float time_so_far = 0.0f;
        for (int i = 0; i < frame_count; ++i) {
            time_so_far += durations[i];
            if (timer <= time_so_far) {
                return i;
            }
        }
        CRASH_NOW_MSG("should not reach here");
        return 0;
    }

    static void updateSpriteAnimation(float dt,
                                      SpriteAnimatorComponent& animator,
                                      SpriteRendererComponent& renderer) {
        SpriteAnimationAsset* asset = animator.animHandle().get();
        if (!asset) {
            return;
        }

        renderer.SetResourceGuid(asset->imageGuid());

        const auto& clip_name = animator.currentClip();
        const auto& clips = asset->clips();
        auto it = clips.find(clip_name);
        if (it == clips.end()) {
            return;
        }
        const SpriteAnimationClip& clip = it->second;

        float timer = animator.playbackTimer();
        if (animator.playing()) {
            timer += dt;
        }

        const float duration = clip.totalDuration();
        if (animator.looping()) {
            timer = std::fmod(timer, duration);
        } else {
            if (timer >= duration) {
                timer = duration;
                animator.playing(false);
            }
        }
        animator.playbackTimer(timer);

        const int frame_idx = getFrame(timer,
                                       duration,
                                       clip.durations());

        DEV_ASSERT_INDEX(frame_idx, clip.frames().size());
        renderer.SetRect(clip.frames()[frame_idx]);
    }
};

void RunSpriteAnimationSystem(Scene& scene, jobsystem::Context&, float dt) {
    auto view = scene.view<SpriteAnimatorComponent, SpriteRendererComponent>();

    for (auto [id, animator, renderer] : view) {
        AnimationSystem::updateSpriteAnimation(dt, animator, renderer);
    }
}

void RunTransformAnimationSystem(Scene& scene, jobsystem::Context&, float dt) {
    auto view = scene.view<TransformAnimationComponent, TransformComponent>();

    std::vector<ecs::Entity> pending_removes;

    for (auto [id, anim, trans] : view) {
        if (!anim.playing) {
            continue;
        }
        anim.elapsed += dt;
        const float t = anim.elapsed / anim.duration;
        if (t >= 1.0f) {
            trans.setTranslation(anim.end);
            anim.playing = false;
            if (anim.destroy_on_finish) {
                pending_removes.push_back(id);
            }
            continue;
        }

        const math::Vec3f pos = t * anim.end + (1 - t) * anim.begin;
        trans.setTranslation(pos);
    }

    for (ecs::Entity e : pending_removes) {
        scene.remove<TransformAnimationComponent>(e);
    }
}

}  // namespace cave
