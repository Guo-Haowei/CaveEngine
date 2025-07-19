#include "sprite_renderer.h"

namespace cave {

bool SpriteRenderer::SetImage(const Guid& p_guid) {
    return AssetHandle::ReplaceGuidAndHandle(AssetType::Image,
                                             p_guid,
                                             image_id,
                                             m_image_handle.RawHandle());
}

void SpriteRenderer::Serialize(Archive& p_archive, uint32_t p_version) {
    unused(p_archive);
    unused(p_version);
}

}  // namespace cave
