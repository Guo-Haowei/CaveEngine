#include "sprite_renderer_component.h"

#include "engine/assets/image_asset.h"
#include "engine/runtime/asset_registry.h"

namespace cave {

void SpriteRendererComponent::SetResourceGuid(const Guid& p_guid) {
    AssetHandle::ReplaceGuidAndHandle(AssetType::Image,
                                      p_guid,
                                      m_image_id,
                                      m_image_handle.RawHandle());
}

void SpriteRendererComponent::OnDeserialized() {
    if (!m_image_id.IsNull()) {
        m_image_handle =
            AssetRegistry::GetSingleton().FindByGuid<ImageAsset>(m_image_id).unwrap();
    }
}

}  // namespace cave
