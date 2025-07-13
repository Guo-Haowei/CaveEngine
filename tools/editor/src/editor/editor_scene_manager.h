#pragma once
#include "engine/scene/scene_manager.h"

namespace my {

using CreateSceneFunc = std::function<std::shared_ptr<Scene>()>;

class EditorSceneManager : public SceneManager {
public:
    virtual Scene* CreateDefaultScene() override;

    void OpenTemporaryScene(std::string_view p_name, const CreateSceneFunc& p_func);

    void DeleteTemporaryScene(const std::string& p_name);

    // @TODO: do not pass raw pointers around
    Scene* GetActiveScene() const override;

protected:
    std::unordered_map<std::string, std::shared_ptr<Scene>> m_caches;

    std::weak_ptr<Scene> m_tmp_scene;
};

}  // namespace my
