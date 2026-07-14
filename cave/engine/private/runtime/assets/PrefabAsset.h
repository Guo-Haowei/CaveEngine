#pragma once
#include "engine/private/runtime/assets/SceneContainer.h"

namespace cave {

class PrefabAsset final : public SceneContainer {
    CAVE_ASSET(PrefabAsset, AssetType::Prefab, 0)

public:
};

}  // namespace cave