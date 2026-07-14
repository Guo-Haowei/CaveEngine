#include "cave/runtime/ui/UIComponents.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

void UIImageComponent::onDeserialized() {
    if (!m_image_guid.isNull()) {
        auto handle = AssetRegistry::singleton().findByGuid<ImageAsset>(m_image_guid);
        if (handle.is_some()) {
            m_image_handle = std::move(handle.unwrap_unchecked());
        }
    }
}

}  // namespace cave
