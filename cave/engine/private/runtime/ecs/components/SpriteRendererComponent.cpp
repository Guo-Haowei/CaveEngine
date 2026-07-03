#include "cave/runtime/ecs/components/SpriteRendererComponent.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

bool SpriteRendererComponent::SetResourceGuid(const Guid& guid) {
    return AssetHandle::replaceGuidAndHandle(AssetType::Image,
                                             guid,
                                             image_id_,
                                             image_handle_.rawHandle());
}

void SpriteRendererComponent::OnDeserialized() {
    if (!image_id_.isNull()) {
        image_handle_ =
            AssetRegistry::singleton().findByGuid<ImageAsset>(image_id_).unwrap();
    }
}

}  // namespace cave
