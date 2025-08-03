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
    Translation,
    Rotation,
    Scale,
    Count,
};

DECLARE_ENUM_TRAITS(AnimationChannelPath, "translation", "rotation", "scale");

struct SkeletalAnimationChannel {
    CAVE_META(SkeletalAnimationChannel)

    CAVE_PROP()
    AnimationChannelPath path = AnimationChannelPath::Count;

    CAVE_PROP()
    ecs::Entity target_id;

    CAVE_PROP()
    int sampler_index = -1;
};

struct SkeletalAnimationSampler {
    CAVE_META(SkeletalAnimationSampler)

    CAVE_PROP()
    std::vector<float> keyframe_times;

    CAVE_PROP()
    std::vector<float> keyframe_data;
};

class SkeletalAnimationComponent {
    CAVE_META(SkeletalAnimationComponent)

private:
    CAVE_PROP(editor = Toggle)
    bool m_playing = false;

    CAVE_PROP(editor = Toggle)
    bool m_looped = true;

    CAVE_PROP(editor = DragFloat, min = 0, max = 1000)
    float m_speed = 1;

    CAVE_PROP()
    float m_start = 0;

    CAVE_PROP()
    float m_end = 0;

    CAVE_PROP()
    float m_timer = 0;

    CAVE_PROP()
    float m_blend_amount = 1;

    CAVE_PROP()
    std::vector<SkeletalAnimationChannel> m_channels;

    CAVE_PROP()
    std::vector<SkeletalAnimationSampler> m_samplers;

    friend class SkeletalAnimationSystem;

public:
    bool IsPlaying() const { return m_playing; }
    void SetPlaying(bool p_value = true) { m_playing = p_value; }

    bool IsLooped() const { return m_looped; }

    float GetLegnth() const { return m_end - m_start; }
    float IsEnd() const { return m_timer > m_end; }

    auto& GetSamplers() { return m_samplers; }
    const auto& GetSamplers() const { return m_samplers; }

    auto& GetChannels() { return m_channels; }
    const auto& GetChannels() const { return m_channels; }

    void SetStart(float p_start) { m_start = p_start; }
    float GetStart() const { return m_start; }

    void SetEnd(float p_end) { m_end = p_end; }
    float GetEnd() const { return m_end; }
};

}  // namespace cave
