#include "SpriteRendererComponent.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

bool SpriteRendererComponent::SetResourceGuid(const Guid& p_guid) {
    return AssetHandle::ReplaceGuidAndHandle(AssetType::Image,
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
