#include "cave/render/components/SpriteRendererComponent.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

void SpriteRendererComponent::setImageGuid(const Guid& guid) {
    if (m_image_id == guid) {
        return;
    }

    m_image_id = guid;
    refreshImageHandle();
}

void SpriteRendererComponent::refreshImageHandle() {
    if (m_image_id.isNull()) {
        m_image_handle = {};
        return;
    }

    auto handle = AssetRegistry::singleton().findByGuid<ImageAsset>(m_image_id);
    if (handle.is_some()) {
        m_image_handle = std::move(handle.unwrap_unchecked());
    }
}

void SpriteRendererComponent::onImageGuidChanged(const FieldChange& change) {
    DEV_ASSERT((*(const Guid*)(change.old_value)) != (*(const Guid*)(change.new_value)));
    DEV_ASSERT(change.object == this);
    DEV_ASSERT(change.field->id == CAVE_SID("image_id"));

    refreshImageHandle();
}

void SpriteRendererComponent::onDeserialized() {
    refreshImageHandle();
}

}  // namespace cave
