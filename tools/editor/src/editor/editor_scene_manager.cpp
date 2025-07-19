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

    ecs::Entity::SetSeed();

    Scene* scene = new Scene;
    auto root = EntityFactory::CreateTransformEntity(*scene, "root");
    scene->m_root = root;

    return scene;
}

void EditorSceneManager::SetTmpScene(const std::shared_ptr<Scene>& p_scene) {
    m_tmp_scene = p_scene;
}

void EditorSceneManager::Update() {
    SceneManager::Update();

    auto it = m_caches.begin();
    for (; it != m_caches.end(); ++it) {
        if (it->second.use_count() == 1) {
            break;
        }
    }

    if (it != m_caches.end()) {
        m_caches.erase(it);
    }
}

void EditorSceneManager::OpenScene(const Guid& p_guid, std::shared_ptr<Scene>& p_scene) {
    auto [_, ok] = m_caches.try_emplace(p_guid, p_scene);
    DEV_ASSERT(ok);

    return;
}

std::shared_ptr<Scene> EditorSceneManager::OpenTemporaryScene(const Guid& p_guid,
                                                              const CreateSceneFunc& p_func) {
    auto scene = p_func();

    auto [_, ok] = m_caches.try_emplace(p_guid, scene);
    DEV_ASSERT(ok);

    return scene;
}

Scene* EditorSceneManager::GetActiveScene() const {
    auto lock = m_tmp_scene.lock();
    if (lock) {
        return lock.get();
    }

    return m_active_scene;
}

}  // namespace cave
