#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"

#include "engine/private/runtime/assets/SpriteAnimationAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

void SpriteAnimatorComponent::currentClip(std::string_view name) {
    SpriteAnimationAsset* asset = anim_handle_.get();
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
    return AssetHandle::replaceGuidAndHandle(AssetType::SpriteAnimation, guid, anim_id_, anim_handle_.rawHandle());
}

void SpriteAnimatorComponent::OnDeserialized() {
    if (!anim_id_.isNull()) {
        anim_handle_ =
            AssetRegistry::singleton().findByGuid<SpriteAnimationAsset>(anim_id_).unwrap();
    }
}

}  // namespace cave
