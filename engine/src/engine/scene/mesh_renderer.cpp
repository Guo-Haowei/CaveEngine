#include "mesh_renderer.h"

#include "engine/core/io/archive.h"

namespace cave {

void MeshRenderer::SetResourceGuid(const Guid& p_guid) {
    unused(p_guid);
    CRASH_NOW();
}

void MeshRenderer::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(flags);
}

}  // namespace cave
