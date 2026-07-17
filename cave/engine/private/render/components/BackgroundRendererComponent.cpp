#include "cave/render/components/BackgroundRendererComponent.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

void BackgroundRendererComponent::refreshImageHandle() {
    if (m_image_guid.isNull()) {
        m_image_handle = {};
        return;
    }

    auto handle = AssetRegistry::singleton().findByGuid<ImageAsset>(m_image_guid);
    if (handle.is_some()) {
        m_image_handle = std::move(handle.unwrap_unchecked());
    }
}

void BackgroundRendererComponent::onImageGuidChanged(const FieldChange& change) {
    DEV_ASSERT((*(const Guid*)(change.old_value)) != (*(const Guid*)(change.new_value)));
    DEV_ASSERT(change.object == this);
    DEV_ASSERT(change.field->id == CAVE_SID("image_guid"));

    refreshImageHandle();
}

void BackgroundRendererComponent::onDeserialized() {
    refreshImageHandle();
}

}  // namespace cave
