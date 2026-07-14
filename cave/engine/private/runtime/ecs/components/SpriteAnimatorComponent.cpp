#include "cave/runtime/ecs/components/SpriteAnimatorComponent.h"

#include "engine/private/runtime/assets/SpriteAnimationAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

void SpriteAnimatorComponent::currentClip(std::string_view name) {
    SpriteAnimationAsset* asset = m_anim_handle.get();
    const SpriteAnimationClip* clip = asset->tryGetClip(std::string(name));
    if (clip == nullptr) {
        LOG_ERROR("Clip '{}' does not exist", name);
        return;
    }

    if (m_current_clip != name) {
        m_current_clip = name;
        m_playback_timer = 0.0f;
        m_looping = clip->looping();
    }
}

bool SpriteAnimatorComponent::setAnimGuid(const Guid& guid) {
    return AssetHandle::replaceGuidAndHandle(AssetType::SpriteAnimation, guid, m_anim_id, m_anim_handle.rawHandle());
}

void SpriteAnimatorComponent::onDeserialized() {
    if (!m_anim_id.isNull()) {
        m_anim_handle =
            AssetRegistry::singleton().findByGuid<SpriteAnimationAsset>(m_anim_id).unwrap();
    }
}

}  // namespace cave
