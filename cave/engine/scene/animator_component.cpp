#include "animator_component.h"

#include "engine/assets/sprite_animation_asset.h"
#include "engine/runtime/asset_registry.h"

namespace cave {

void AnimatorComponent::SetClip(const std::string& p_name) {
    if (m_current_clip != p_name) {
        m_current_clip = p_name;
        m_playback_timer = 0.0f;
    }
}

bool AnimatorComponent::SetResourceGuid(const Guid& p_guid) {
    return AssetHandle::ReplaceGuidAndHandle(AssetType::SpriteAnimation, p_guid, m_anim_id, m_anim_handle.RawHandle());
}

void AnimatorComponent::OnDeserialized() {
    if (!m_anim_id.IsNull()) {
        m_anim_handle =
            AssetRegistry::GetSingleton().FindByGuid<SpriteAnimationAsset>(m_anim_id).unwrap();
    }
}

}  // namespace cave
