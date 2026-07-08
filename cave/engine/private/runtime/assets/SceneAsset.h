#pragma once
#include "cave/core/error/ErrorMacros.h"
#include "cave/runtime/assets/IAsset.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

class SceneAsset final : public IAsset {
    CAVE_ASSET(SceneAsset, AssetType::Scene, 0)

public:
    SceneAsset();
    ~SceneAsset() override;

    Scene& sceneMut() noexcept {
        DEV_ASSERT(m_scene);
        return *m_scene;
    }

    const Scene& scene() const noexcept {
        DEV_ASSERT(m_scene);
        return *m_scene;
    }

    auto loadFromDisk(const AssetMetaData&) -> Result<void> override;
    auto saveToDisk(const AssetMetaData&) const -> Result<void> override;
    virtual Vector<Guid> dependencies() const override;

private:
    Owner<Scene> m_scene;
};

}  // namespace cave
