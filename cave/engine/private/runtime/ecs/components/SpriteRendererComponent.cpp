#include "cave/runtime/ecs/components/SpriteRendererComponent.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

bool SpriteRendererComponent::SetResourceGuid(const Guid& guid) {
    return AssetHandle::replaceGuidAndHandle(AssetType::Image,
                                             guid,
                                             m_image_id,
                                             m_image_handle.rawHandle());
}

void SpriteRendererComponent::onDeserialized() {
    if (!m_image_id.isNull()) {
        m_image_handle =
            AssetRegistry::singleton().findByGuid<ImageAsset>(m_image_id).unwrap();
    }
}

}  // namespace cave
