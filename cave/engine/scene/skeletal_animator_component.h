#pragma once
#include "engine/ecs/entity.h"
#include "engine/math/geomath.h"
#include "engine/reflection/reflection.h"

namespace cave {

struct SkeletonComponent {
    CAVE_META(SkeletonComponent)

    CAVE_PROP()
    std::vector<ecs::Entity> bone_collection;

    CAVE_PROP()
    std::vector<Matrix4x4f> inverse_bind_matrices;

    // Non-Serialized
    std::vector<Matrix4x4f> bone_transforms;
};

enum class AnimationChannelPath {
    Unknown,
    Translation,
    Rotation,
    Scale,
};

class SkeletalAnimationComponent {
    CAVE_META(SkeletalAnimationComponent)

private:
    CAVE_PROP(editor = Toggle)
    bool m_playing = true;

    CAVE_PROP(editor = Toggle)
    bool m_looped = true;

public:
    struct Channel {
        AnimationChannelPath path = AnimationChannelPath::Unknown;
        ecs::Entity targetId;
        int samplerIndex = -1;
    };

    struct Sampler {
        std::vector<float> keyframeTimes;
        std::vector<float> keyframeData;
    };

    bool IsPlaying() const { return m_playing; }
    void SetPlaying(bool p_value = true) { m_playing = p_value; }

    bool IsLooped() const { return m_looped; }

    float GetLegnth() const { return end - start; }
    float IsEnd() const { return timer > end; }

    float start = 0;
    float end = 0;
    float timer = 0;
    float amount = 1;  // blend amount
    float speed = 1;

    std::vector<Channel> channels;
    std::vector<Sampler> samplers;
};

}  // namespace cave
