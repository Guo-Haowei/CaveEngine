#include "SceneAsset.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

auto SceneAsset::loadFromDisk(const AssetMetaData& meta) -> Result<void> {
    return SceneContainer::loadFromDisk(meta);
}

}  // namespace cave