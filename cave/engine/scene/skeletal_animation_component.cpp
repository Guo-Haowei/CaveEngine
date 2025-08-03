#include "skeletal_animation_component.h"

#include "engine/serialization/yaml_include.h"

namespace cave {
//
// ISerializer& WriteObject(ISerializer& s, const SkeletalAnimationComponent::SkeletalAnimationChannel& p_channel) {
//    unused(p_channel);
//    return s;
//}
//
// bool ReadObject(IDeserializer& d, SkeletalAnimationComponent::SkeletalAnimationChannel& p_channel) {
//    unused(d);
//    unused(p_channel);
//    return false;
//}
//
// ISerializer& WriteObject(ISerializer& s, const SkeletalAnimationComponent::SkeletalAnimationSampler& p_sampler) {
//    unused(p_sampler);
//    return s;
//}
//
// bool ReadObject(IDeserializer& d, SkeletalAnimationComponent::SkeletalAnimationSampler& p_sampler) {
//    unused(d);
//    unused(p_sampler);
//    return false;
//}

#if 0
void AnimationComponent::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(flags);
    p_archive.ArchiveValue(start);
    p_archive.ArchiveValue(end);
    p_archive.ArchiveValue(timer);
    p_archive.ArchiveValue(amount);
    p_archive.ArchiveValue(speed);
    p_archive.ArchiveValue(channels);

    if (p_archive.IsWriteMode()) {
        uint64_t num_samplers = samplers.size();
        p_archive << num_samplers;
        for (uint64_t i = 0; i < num_samplers; ++i) {
            p_archive << samplers[i].keyframeTimes;
            p_archive << samplers[i].keyframeData;
        }
    } else {
        uint64_t num_samplers = 0;
        p_archive >> num_samplers;
        samplers.resize(num_samplers);
        for (uint64_t i = 0; i < num_samplers; ++i) {
            p_archive >> samplers[i].keyframeTimes;
            p_archive >> samplers[i].keyframeData;
        }
    }
}

#endif
}  // namespace cave
