#include "SceneAsset.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

auto SceneAsset::loadFromDisk(const AssetMetaData& meta) -> Result<void> {
    auto result = SceneContainer::loadFromDisk(meta);
    if (!result) {
        return result;
    }

    DEV_ASSERT(m_scene);

    if (m_scene->version() <= 19) {
        auto ent = m_scene->root();
        if (DEV_VERIFY(!m_scene->has(HierarchyComponent_Id, ent))) {
            m_scene->create(HierarchyComponent_Id, ent);
        }
    }

    return Result<void>();
}

}  // namespace cave