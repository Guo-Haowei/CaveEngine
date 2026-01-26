#include "EditorSceneManager.h"

#include "EditorDvars.h"
#include "engine/private/runtime/scene/EntityFactory.h"

namespace cave {

#if 0
void EditorSceneManager::Update() {
    SceneManager::Update();

    auto it = m_scenes.begin();
    for (; it != m_scenes.end(); ++it) {
        if (it->second.scene.use_count() == 1) {
            break;
        }
    }

    if (it != m_scenes.end()) {
        LOG_VERBOSE("Unloading scene '{}'...", it->first);
        m_scenes.erase(it);
    }
}
#endif

void EditorSceneManager::OpenScene(const Guid& p_guid, std::shared_ptr<Scene>& p_scene) {
    std::string id = p_guid.ToString();
    auto [_, ok] = m_scenes.try_emplace(id, SceneHandle{ SceneType::Disk, p_scene });
    DEV_ASSERT(ok);
    return;
}

std::shared_ptr<Scene> EditorSceneManager::CreateTempScene(const Guid& p_guid,
                                                           const CreateSceneFunc& p_func) {
    std::string id = p_guid.ToString();
    DEV_ASSERT(m_scenes.find(id) == m_scenes.end());

    auto scene = p_func();

    m_scenes.insert({ std::move(id), { SceneType::Temp, scene } });
    return scene;
}

}  // namespace cave
