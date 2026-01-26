#pragma once
#include "engine/private/assets/guid.h"
#include "engine/private/runtime/scene/SceneManager.h"

namespace cave {

using CreateSceneFunc = std::function<std::shared_ptr<Scene>()>;

class EditorSceneManager : public SceneManager {
public:
    std::shared_ptr<Scene> CreateTempScene(const Guid& p_guid,
                                           const CreateSceneFunc& p_func);

    void OpenScene(const Guid& p_guid, std::shared_ptr<Scene>& p_scene);

    void OpenSimScene(const std::shared_ptr<Scene>& p_scene) override;

    void CloseSimScene() override;

    std::shared_ptr<Scene> GetActiveScene() const override;

    void Update() override;

protected:
    enum class SceneType : uint8_t {
        Temp,
        Disk,
        Sim,
    };

    struct SceneHandle {
        SceneType type;
        std::shared_ptr<Scene> scene;
    };

    std::unordered_map<std::string, SceneHandle> m_scenes;

    std::weak_ptr<Scene> m_tmp_scene;
    std::shared_ptr<Scene> m_sim_scene;
};

}  // namespace cave
