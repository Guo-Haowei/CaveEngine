#include "animator_component.h"

#include "engine/assets/sprite_animation_asset.h"
#include "engine/runtime/asset_registry.h"

namespace cave {

void SpriteAnimatorComponent::SetClip(const std::string& p_name) {
    if (m_current_clip != p_name) {
        m_current_clip = p_name;
        m_playback_timer = 0.0f;
    }
}

bool SpriteAnimatorComponent::SetResourceGuid(const Guid& p_guid) {
    return AssetHandle::ReplaceGuidAndHandle(AssetType::SpriteAnimation, p_guid, m_anim_id, m_anim_handle.RawHandle());
}

void SpriteAnimatorComponent::OnDeserialized() {
    if (!m_anim_id.IsNull()) {
        m_anim_handle =
            AssetRegistry::GetSingleton().FindByGuid<SpriteAnimationAsset>(m_anim_id).unwrap();
    }
}

}  // namespace cave
