#include "animator_component.h"

#include "engine/assets/sprite_animation_asset.h"
#include "engine/runtime/asset_registry.h"

namespace cave {

void AnimatorComponent::SetClip(const std::string& p_name, bool p_looping, float p_duration) {
    m_current_clip = p_name;
    m_looping = p_looping;
    m_playing = true;

    m_playback_timer.start = 0.0f;
    m_playback_timer.end = p_duration;
    m_playback_timer.timer = 0.0f;
}

void AnimatorComponent::SetResourceGuid(const Guid& p_guid) {
    AssetHandle::ReplaceGuidAndHandle(AssetType::SpriteAnimation, p_guid, m_anim_id, m_anim_handle.RawHandle());
}

void AnimatorComponent::OnDeserialized() {
    if (!m_anim_id.IsNull()) {
        m_anim_handle =
            AssetRegistry::GetSingleton().FindByGuid<SpriteAnimationAsset>(m_anim_id).unwrap();
    }
}

void AnimatorComponent::Serialize(Archive& p_archive, uint32_t p_version) {
    unused(p_archive);
    unused(p_version);
}

}  // namespace cave
