#pragma once
#include "engine/runtime/scene_manager_interface.h"
#include "engine/core/base/concurrent_queue.h"

namespace my {

class Scene;
class Application;

#if 0
class SceneManager {
public:
    // Load and set as active scene
    Result<void> LoadScene(const std::string& path);
    Result<void> LoadSceneAsync(const std::string& path);

    // Save current scene
    Result<void> SaveScene(const std::string& path);

    // Unload current scene
    void UnloadScene();

    // Reload current scene from file
    Result<void> ReloadScene();

    // Access current active scene
    Scene* GetActiveScene();
    std::shared_ptr<Scene> GetActiveSceneShared();

    // Create a new empty scene (editor or runtime)
    std::shared_ptr<Scene> CreateNewScene();

    // Instantiate a scene (like a prefab or instance-in-level)
    std::shared_ptr<SceneInstance> InstantiateScene(const std::string& path);

    // Editor: mark scene dirty/clean
    void MarkDirty(bool value);
    bool IsDirty() const;

private:
    std::shared_ptr<Scene> active_scene;
    bool dirty = false;
};
#endif

class SceneManager : public ISceneManager {
public:
    SceneManager()
        : ISceneManager("SceneManager") {}

    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;
    void Update() override;

    Scene* GetActiveScene() const override;
#if 0
    void RequestScene(std::string_view p_path);


    uint32_t GetRevision() const { return m_revision; }
#endif
    void BumpRevision() { ++m_revision; }

    virtual Scene* CreateDefaultScene();

    // void EnqueueSceneLoadingTask(Scene* p_scene, bool p_replace);

protected:
    bool TrySwapScene();

    Scene* m_active_scene = nullptr;

    uint32_t m_revision = 0;
    uint32_t m_lastRevision = 0;

    struct LoadSceneTask {
        bool replace;
        Scene* scene;
    };

    ConcurrentQueue<LoadSceneTask> m_loadingQueue;
};

}  // namespace my
