#include "sprite_renderer.h"

#include "engine/assets/image_asset.h"
#include "engine/runtime/asset_registry.h"

namespace cave {

bool SpriteRenderer::SetResourceGuid(const Guid& p_guid) {
    return AssetHandle::ReplaceGuidAndHandle(AssetType::Image,
                                             p_guid,
                                             m_image_id,
                                             m_image_handle.RawHandle());
}

void SpriteRenderer::OnDeserialized() {
    if (!m_image_id.IsNull()) {
        m_image_handle =
            AssetRegistry::GetSingleton().FindByGuid<ImageAsset>(m_image_id).unwrap();
    }
}

void SpriteRenderer::Serialize(Archive& p_archive, uint32_t p_version) {
    unused(p_archive);
    unused(p_version);
}

}  // namespace cave
