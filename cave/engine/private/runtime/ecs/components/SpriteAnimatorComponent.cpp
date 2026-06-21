#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"

#include "engine/private/runtime/assets/SpriteAnimationAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

void SpriteAnimatorComponent::currentClip(std::string_view name) {
    SpriteAnimationAsset* asset = anim_handle_.Get();
    const SpriteAnimationClip* clip = asset->tryGetClip(std::string(name));
    if (clip == nullptr) {
        LOG_ERROR("Clip '{}' does not exist", name);
        return;
    }

    if (current_clip_ != name) {
        current_clip_ = name;
        playback_timer_ = 0.0f;
        looping_ = clip->looping();
    }
}

bool SpriteAnimatorComponent::SetResourceGuid(const Guid& guid) {
    return AssetHandle::ReplaceGuidAndHandle(AssetType::SpriteAnimation, guid, anim_id_, anim_handle_.RawHandle());
}

void SpriteAnimatorComponent::OnDeserialized() {
    if (!anim_id_.IsNull()) {
        anim_handle_ =
            AssetRegistry::singleton().FindByGuid<SpriteAnimationAsset>(anim_id_).unwrap();
    }
}

}  // namespace cave
