#pragma once
#include "cave/core/error/ErrorMacros.h"
#include "cave/runtime/assets/IAsset.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

class SceneContainer : public IAsset {
public:
    SceneContainer();
    ~SceneContainer() override;

    Scene& sceneMut() noexcept {
        DEV_ASSERT(m_scene);
        return *m_scene;
    }

    const Scene& scene() const noexcept {
        DEV_ASSERT(m_scene);
        return *m_scene;
    }

    auto loadFromDisk(const AssetMetaData& meta) -> Result<void> override;
    auto saveToDisk(const AssetMetaData& meta) const -> Result<void> override;
    virtual Vector<Guid> dependencies() const override;

protected:
    Owner<Scene> m_scene;
};

}  // namespace cave
