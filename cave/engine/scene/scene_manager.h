#pragma once
#include "engine/runtime/scene_manager_interface.h"

namespace cave {

class Scene;

class SceneManager : public ISceneManager {
public:
    SceneManager()
        : ISceneManager("SceneManager") {}

    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;
    void Update() override;

    std::shared_ptr<Scene> GetActiveScene() const override;

    void BumpRevision() override { ++m_revision; }

    virtual Scene* CreateDefaultScene();

protected:
    bool TrySwapScene();

    uint32_t m_revision = 0;
    uint32_t m_lastRevision = 0;

    struct LoadSceneTask {
        bool replace;
        Scene* scene;
    };
};

}  // namespace cave
