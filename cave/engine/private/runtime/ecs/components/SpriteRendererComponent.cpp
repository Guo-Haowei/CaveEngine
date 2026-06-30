#include "cave/runtime/ecs/components/SpriteRendererComponent.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

bool SpriteRendererComponent::SetResourceGuid(const Guid& p_guid) {
    return AssetHandle::replaceGuidAndHandle(AssetType::Image,
                                             p_guid,
                                             image_id_,
                                             image_handle_.rawHandle());
}

void SpriteRendererComponent::OnDeserialized() {
    if (!image_id_.IsNull()) {
        image_handle_ =
            AssetRegistry::singleton().findByGuid<ImageAsset>(image_id_).unwrap();
    }
}

}  // namespace cave
