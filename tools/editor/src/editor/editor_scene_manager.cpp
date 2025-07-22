#include "editor_scene_manager.h"

#include "editor_dvars.h"
#include "engine/scene/entity_factory.h"

namespace cave {

extern Scene* CreateTheAviatorScene();
extern Scene* CreateBoxScene();
extern Scene* CreatePbrTestScene();
extern Scene* CreatePhysicsTestScene();

Scene* EditorSceneManager::CreateDefaultScene() {
    auto scene_string = DVAR_GET_STRING(default_scene);
    if (scene_string == "pbr_test") {
        return CreatePbrTestScene();
    }
    if (scene_string == "physics_test") {
        return CreatePhysicsTestScene();
    }
    if (scene_string == "the_aviator") {
        return CreateTheAviatorScene();
    }
    if (scene_string == "box") {
        return CreateBoxScene();
    }

    Scene* scene = new Scene;
    auto root = EntityFactory::CreateTransformEntity(*scene, "root");
    scene->m_root = root;

    return scene;
}

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

void EditorSceneManager::OpenScene(const Guid& p_guid, std::shared_ptr<Scene>& p_scene) {
    std::string id = p_guid.ToString();
    auto [_, ok] = m_scenes.try_emplace(id, SceneHandle{ SceneType::Disk, p_scene });
    DEV_ASSERT(ok);
    return;
}

void EditorSceneManager::OpenSimScene(const std::shared_ptr<Scene>& p_scene) {
    m_sim_scene = p_scene;
}

void EditorSceneManager::CloseSimScene() {
    m_sim_scene.reset();
}

void EditorSceneManager::OpenTempScene(const std::shared_ptr<Scene>& p_scene) {
    m_tmp_scene = p_scene;
}

std::shared_ptr<Scene> EditorSceneManager::CreateTempScene(const Guid& p_guid,
                                                           const CreateSceneFunc& p_func) {
    std::string id = p_guid.ToString();
    DEV_ASSERT(m_scenes.find(id) == m_scenes.end());

    auto scene = p_func();

    m_scenes.insert({ std::move(id), { SceneType::Temp, scene } });
    return scene;
}

std::shared_ptr<Scene> EditorSceneManager::GetActiveScene() const {
    if (auto lock = m_sim_scene.lock(); lock) {
        return lock;
    }

    if (auto lock = m_tmp_scene.lock(); lock) {
        return lock;
    }

    return nullptr;
}

}  // namespace cave
