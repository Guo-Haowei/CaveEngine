#pragma once
#include "engine/assets/guid.h"
#include "engine/scene/scene_manager.h"

namespace cave {

using CreateSceneFunc = std::function<std::shared_ptr<Scene>()>;

class EditorSceneManager : public SceneManager {
public:
    virtual Scene* CreateDefaultScene() override;

    [[nodiscard]]
    std::shared_ptr<Scene> OpenTemporaryScene(const Guid& p_guid,
                                              const CreateSceneFunc& p_func);

    // @TODO: do not pass raw pointers around
    Scene* GetActiveScene() const override;

    void SetTmpScene(const std::shared_ptr<Scene>& p_scene);

    void Update() override;

protected:
    std::unordered_map<Guid, std::shared_ptr<Scene>> m_caches;

    std::weak_ptr<Scene> m_tmp_scene;
};

}  // namespace cave
