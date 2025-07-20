#include "mesh_renderer.h"

#include "engine/core/io/archive.h"

namespace cave {

void MeshRenderer::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(flags);
    p_archive.ArchiveValue(meshId);
}

}  // namespace cave
