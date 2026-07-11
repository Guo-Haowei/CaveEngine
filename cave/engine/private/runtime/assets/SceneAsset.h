#pragma once
#include "engine/private/runtime/assets/SceneContainer.h"

namespace cave {

class SceneAsset final : public SceneContainer {
    CAVE_ASSET(SceneAsset, AssetType::Scene, 0)
};

}  // namespace cave
