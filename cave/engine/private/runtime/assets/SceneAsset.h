#pragma once
#include "engine/private/runtime/assets/SceneContainer.h"

namespace cave {

class SceneAsset final : public SceneContainer {
    CAVE_ASSET(SceneAsset, AssetType::Scene, 21)

public:
    auto loadFromDisk(const AssetMetaData& meta) -> Result<void> override;
};

}  // namespace cave
