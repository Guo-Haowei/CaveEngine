#include "editor_scene_manager.h"

#include "editor_dvars.h"

namespace my {

extern Scene* CreateTheAviatorScene();
extern Scene* CreateBoxScene();
extern Scene* CreatePbrTestScene();
extern Scene* CreatePhysicsTestScene();

Scene* EditorSceneManager::CreateDefaultScene() {
    auto scene = DVAR_GET_STRING(default_scene);
    if (scene == "pbr_test") {
        return CreatePbrTestScene();
    }
    if (scene == "physics_test") {
        return CreatePhysicsTestScene();
    }
    if (scene == "the_aviator") {
        return CreateTheAviatorScene();
    }
    if (scene == "box") {
        return CreateBoxScene();
    }
    return nullptr;
}

void EditorSceneManager::OpenTemporaryScene(std::string_view p_name, const CreateSceneFunc& p_func) {
    auto scene = p_func();

    m_tmp_scene = scene;

    m_caches.insert({ std::string(p_name), std::move(scene) });
}

void EditorSceneManager::DeleteTemporaryScene(const std::string& p_name) {
    m_caches.erase(p_name);
}

Scene* EditorSceneManager::GetActiveScene() const {
    auto lock = m_tmp_scene.lock();
    if (lock) {
        return lock.get();
    }

    return nullptr;
}

}  // namespace my

