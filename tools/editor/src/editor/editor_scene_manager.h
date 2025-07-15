#pragma once
#include "engine/scene/scene_manager.h"

namespace cave {

using CreateSceneFunc = std::function<std::shared_ptr<Scene>()>;

class EditorSceneManager : public SceneManager {
public:
    virtual Scene* CreateDefaultScene() override;

    void OpenTemporaryScene(std::string_view p_name, const CreateSceneFunc& p_func);

    void DeleteTemporaryScene(const std::string& p_name);

    // @TODO: do not pass raw pointers around
    Scene* GetActiveScene() const override;

    void Update() override;

protected:
    struct StaleScene {
        std::shared_ptr<Scene> scene;
        int frame_left;
    };

    std::unordered_map<std::string, std::shared_ptr<Scene>> m_caches;

    std::weak_ptr<Scene> m_tmp_scene;
    std::vector<StaleScene> m_stale_scenes;
};

}  // namespace cave
